#!/bin/python3

import discord
from discord import app_commands
from discord.ext import tasks

import openai
import os
import jsons
import traceback
import gtts
import sys
import urllib.request

# Local modules
import brain.memory as ai_memory


class RunConfig:
    discord_token: str = ""
    openai_token: str = ""
    username: str = ""
    super_user: int = 363144375468949546
    status_prompt: str = 'Come up with a unique and random completion to the phrase "Watching ..." try to exclude the words "I’m" and "Watching"'
    status_model: str = 'text-davinci-002'


class ChannelSettings:
    channel_name: str = "Unknown Channel"
    before: str = "Respond to"
    after: str = ""
    address: bool = True
    remember: bool = True


class AISettings:
    before: str = "Respond to"
    after: str = ""
    address: bool = True
    remember: bool = True

    model: str = "text-curie-001"
    max_tokens: int = 256
    temp: float = 1.0

    channel_id: int = -1
    announcement: str = "Hello! @everyone"

    whitelist_only: bool = False
    allowed_channels: list[int] = []

    nickname: bool = False
    nickname_prompt: str = "Come up with a unique and random name for yourself"

    unique_settings: dict[int, ChannelSettings] = {}


#
# Globals
#
intents = discord.Intents().all()
bot: discord.Client = discord.Client(intents=intents)
tree: app_commands.CommandTree = app_commands.CommandTree(bot)
talk_to_self = False

config_folder = "config"
brain_path = f"{config_folder}/brain.json"
run_path = f"{config_folder}/run.json"

temp_folder = "temp"

#
# Inputs
#
voice_channel: discord.VoiceChannel = None
voice_client: discord.VoiceClient = None
tts_lang = "en"
tts_tld = "us"


#
# Helpers
#
def validate_path(path):
    if not os.path.exists(path):
        os.mkdir(path)


#
# Brain
#
class Brain:
    user_memory: dict[int, ai_memory.Memory] = {}
    guild_memory: dict[int, dict[int, dict[int, ai_memory.Memory]]] = {}

    user_settings: dict[int, AISettings] = {}
    guild_settings: dict[int, AISettings] = {}

    def write(self):
        json = jsons.dumps(self, jdkwargs={"indent": 4})
        validate_path(config_folder)
        with open(brain_path, "w") as f:
            f.write(json)

    def remember_user(self, user: int, what: str):
        if user not in self.user_memory:
            self.user_memory[user] = ai_memory.Memory()

        self.user_memory[user].what = what
        print(f"USER MEMORY: {user} = {what}")

    def remember_guild(self, guild: int, channel: int, user: int, what: str):
        if guild not in self.guild_memory:
            self.guild_memory[guild] = {}

        if channel not in brain.guild_memory[guild]:
            self.guild_memory[guild][channel] = {}

        if user not in brain.guild_memory[guild][channel]:
            self.guild_memory[guild][channel][user] = ai_memory.Memory()

        self.guild_memory[guild][channel][user].what = what
        print(f"GUILD MEMORY: {guild} -> {channel} -> {user} = {what}")

    def remember(self, guild: int | None, channel: int | None, user: int, what: str):
        if guild is None:
            self.remember_user(user, what)
        else:
            self.remember_guild(guild, channel, user, what)

    def recall(self, guild: int | None, user: int) -> ai_memory.Memory | None:
        if guild is None:
            if user in self.user_memory:
                return self.user_memory[user]
        else:
            if guild in self.guild_memory:
                if user in self.guild_memory[guild]:
                    return self.guild_memory[guild][user]

        return None

    def get_settings(self, lookup: int, is_guild: bool) -> AISettings:
        if not is_guild:
            if lookup is bot.user.id:
                return AISettings()

            if lookup not in self.user_settings:
                self.user_settings[lookup] = AISettings()

            return self.user_settings[lookup]
        else:
            if lookup not in self.guild_settings:
                self.guild_settings[lookup] = AISettings()

            return self.guild_settings[lookup]

    def set_settings(self, lookup: int, is_guild: bool, settings: AISettings):
        if not is_guild:
            self.user_settings[lookup] = settings
        else:
            self.guild_settings[lookup] = settings

        self.write()

    @staticmethod
    def ask_openai_raw(prompt: str, model: str, max_tokens: int, temp: float):
        return openai.Completion.create(
            engine=model,
            max_tokens=max_tokens,
            temperature=temp,

            prompt=prompt,
        ).to_dict()

    def ask_openai(self, lookup: int, is_guild: bool, prompt: str) -> str:
        settings = self.get_settings(lookup, is_guild)

        completion = self.ask_openai_raw(prompt, settings.model, settings.max_tokens, settings.temp)

        responses = ""
        for choice in completion["choices"]:
            responses += choice.text

        return responses

    async def send_ann(self, content: str, embed: discord.Embed):
        global bot

        for guild in bot.guilds:
            if guild.id in self.guild_settings:
                settings = self.guild_settings[guild.id]
    
                if settings.channel_id != -1:
                    ann = guild.get_channel(settings.channel_id)
    
                    if ann is not None:
                        await ann.send(content, embed=embed)

    @staticmethod
    async def generate_action() -> str:
        if testing_mode:
            return "DEBUG MODE! 🔥🔥🔥"
        else:
            completion = brain.ask_openai_raw(
                run_config.status_prompt,
                run_config.status_model,
                32,
                1.0)

            responses = ""
            for choice in completion["choices"]:
                responses += choice.text

            responses = responses.strip()
            responses = responses.strip("\"'.?!")
            responses = responses[:128]
            responses = responses.lower()

            return responses

    @staticmethod
    async def generate_nickname(guild: discord.Guild, prompt: str) -> str:
        nickname = ""
        completion = brain.ask_openai_raw(
            prompt,
            "text-davinci-002",
            16,
            1.0)

        for choice in completion["choices"]:
            nickname += choice.text

        nickname = nickname.strip()
        nickname = nickname.strip("\"'.?!")
        nickname = nickname[:32]

        bot_self = guild.get_member(bot.user.id)

        if bot_self is not None:
            print(f"CLIENT: Nickname in '{guild}' = '{nickname}'")
            await bot_self.edit(nick=nickname)

        return nickname


#
# More globals
#
run_config: RunConfig = RunConfig()
brain: Brain = Brain()
testing_mode = "TESTING_MODE" in os.environ or "--test-mode" in sys.argv
embed_color = discord.Color.from_rgb(67, 160, 71)


@bot.event
async def on_ready():
    global bot
    global brain

    print(f"CLIENT: Logged in as bot user {bot.user.name}")

    if run_config.username and run_config.username != bot.user.name:
        print(f"CLIENT: Attempting to change username to {run_config.username}")
        await bot.user.edit(username=run_config.username)

    for guild in bot.guilds:
        print(f"CLIENT: In server '{guild.name}'")

    validate_path(config_folder)

    if os.path.exists(brain_path):
        with open(brain_path, "r") as f:
            raw = f.read()
            brain = jsons.loads(raw, cls=Brain)

    await tree.sync()
    generate_new_status.start()

    for guild in bot.guilds:
        if guild.id in brain.guild_settings:
            nickname = "NOT ENABLED"

            if brain.guild_settings[guild.id].nickname:
                nickname = await brain.generate_nickname(guild, brain.guild_settings[guild.id].nickname_prompt)

            embed = discord.Embed(title="Online", description="")
            embed.color = embed_color

            embed.add_field(name="Server Nickname", value=nickname, inline=False)

            settings = brain.guild_settings[guild.id]

            if settings.channel_id != -1:
                ann = guild.get_channel(settings.channel_id)

                if ann is not None:
                    await ann.send("", embed=embed)

@bot.event
async def on_message(message: discord.Message):
    global brain
    global talk_to_self

    #await bot.process_commands(message)

    try:
        is_dm = (message.channel.type is discord.ChannelType.private and message.author.id != bot.user.id)
        is_thread = (message.channel.type is discord.ChannelType.public_thread and message.author.id != bot.user.id)
        is_reply = False
        is_self = message.author.id == bot.user.id
        is_guild = message.guild is not None

        guild_id = None
        lookup = message.author.id

        if message.guild is not None:
            guild_id = message.guild.id
            lookup = guild_id

        predicate = ""

        has_reference = message.reference is not None
        settings = brain.get_settings(lookup, is_guild)

        channel_id = message.channel.id

        if is_thread:
            channel_id = message.channel.parent.id

        if channel_id not in settings.allowed_channels and settings.whitelist_only:
            if not testing_mode:
                return
            else:
                if not is_self:
                    await message.channel.send("**Non-allowed channel countered!** Skipping it...")

                return

        if message.is_system():
            if not testing_mode:
                return
            else:
                await message.channel.send("**System message encountered!** Skipping it...")
                return

        if len(message.content) == 0:
            if not testing_mode:
                return
            else:
                await message.channel.send("**Blank message!** Skipping it...")
                return

        if has_reference:
            msg = message.reference.resolved

            if msg is not None:
                if msg.is_system():
                    if not testing_mode:
                        return
                    else:
                        await message.channel.send("**System message encountered!** Skipping it...")
                        return

                is_reply = msg.author.id == bot.user.id

                if is_reply:
                    predicate = f"Remember, you last said '{msg.content}'\n"
            else:
                has_reference = True

        recipient = message.author.name

        if message.author is discord.Member:
            recipient = message.author.nick

        implicit_reply = is_dm or is_reply or is_thread or is_self
        blocked_reply = is_self and not talk_to_self

        if f"{bot.user.id}" in message.content or implicit_reply and not blocked_reply:
            # We need to sanitize the mention of the bot
            mention_self = f"<@{bot.user.id}>"
            sanitized_message = message.content.replace(mention_self, "")
            sanitized_message = sanitized_message.strip()

            # Check if this channel has a unique batch of settings
            composure = settings

            if is_thread and message.channel.id in settings.unique_settings:
                composure = settings.unique_settings[message.channel.id]

                # Used for manual JSON modification
                composure.channel_name = message.channel.name

            if not has_reference:
                mem = brain.recall(guild_id, message.author.id)

                if mem is not None and composure.remember:
                    predicate = mem.get_memory()

            if composure.address:
                predicate += f"Address the recipient of the conversation as '{recipient}'\n"

            prompt = f"{predicate}{composure.before} '{sanitized_message}' {composure.after}"

            print(f"SELF: {mention_self}")

            if message.guild is not None:
                print(f"GUILD: {message.guild} ({message.guild.id})")
            else:
                print("IS A DM!")

            print(f"ASKER: {message.author.name}")
            print(f"OPENAI: Responding to {message.content}")
            print(f"SANITIZED: {sanitized_message}")
            print(f"PROMPT: {prompt}")

            # We find every mention in the message beforehand
            # TODO

            raw_response = brain.ask_openai(lookup, is_guild, prompt)
            response = raw_response.strip()

            print(f"RESPONSE: {response}")
            await message.reply(response, mention_author=True)

            if voice_channel is not None and voice_client is not None:
                validate_path(temp_folder)
                if os.path.exists(f"{temp_folder}/temp.mp3"):
                    os.remove(f"{temp_folder}/temp.mp3")

                if voice_client.is_playing():
                    voice_client.stop()

                tts_reply = f"Hey {message.author.name}, {response}"
                tts = gtts.gTTS(text=tts_reply, lang=tts_lang, tld=tts_tld, lang_check=False, slow=False)
                tts.save(f"{temp_folder}/temp.mp3")

                voice_client.play(discord.FFmpegPCMAudio(f"{temp_folder}/temp.mp3"))

            brain.remember(guild_id, message.channel.id, message.author.id, response)
            print("\n")

            if testing_mode:
                embed = discord.Embed(title="Context", description="")
                embed.color = discord.Color.from_rgb(255, 138, 101)

                embed.add_field(name="Self", value=mention_self, inline=False)
                embed.add_field(name="Guild", value=message.guild, inline=False)
                embed.add_field(name="Asker", value=message.author, inline=False)

                await message.reply(embed=embed)

                embed = discord.Embed(title="AI IO", description="")
                embed.color = discord.Color.from_rgb(255, 138, 101)

                embed.add_field(name="Input", value=prompt, inline=False)
                embed.add_field(name="Output", value=response, inline=False)

                await message.channel.send(embed=embed)

                embed = discord.Embed(title="Prompt Composure", description="")
                embed.color = discord.Color.from_rgb(255, 138, 101)

                if predicate:
                    embed.add_field(name="Predicate", value=predicate, inline=False)

                if settings.before:
                    embed.add_field(name="Before", value=settings.before, inline=False)

                embed.add_field(name="Content", value=message.content, inline=False)

                if settings.after:
                    embed.add_field(name="After", value=settings.after, inline=False)

                await message.channel.send(embed=embed)

                embed = discord.Embed(title="Raw Info", description="")
                embed.color = discord.Color.from_rgb(255, 138, 101)

                embed.add_field(name="AI Settings", value=jsons.dumps(settings, jdkwargs={"indent": 4}), inline=False)
                embed.add_field(name="Is DM?", value=is_dm)
                embed.add_field(name="Is Reply", value=is_reply)
                embed.add_field(name="Is Self?", value=is_self)
                embed.add_field(name="Is Thread?", value=is_thread)

                await message.channel.send(embed=embed)
    except Exception as e:
        tb = traceback.format_exc()
        await message.channel.send(f"ERROR!\n\n{tb}")

    # We update configs per message
    brain.write()


#
# Bot tasks
#
@tasks.loop(hours=1)
async def generate_new_status():
    global brain

    action = await brain.generate_action()

    print(f"CLIENT: Status = {action}")
    await bot.change_presence(activity=discord.Activity(type=discord.ActivityType.watching, name=action), status=discord.Status.online)

    embed = discord.Embed(title="New Status", description=f"Watching {action}")
    embed.color = embed_color

    for guild in bot.guilds:
        if guild.id in brain.guild_settings:
            if brain.guild_settings[guild.id].channel_id != -1:
                ann = guild.get_channel(brain.guild_settings[guild.id].channel_id)

                if ann is not None:
                    await ann.send("", embed=embed)


#
# Bot commands
#
def get_lookup(ctx: discord.Interaction) -> int:
    if ctx.guild is None:
        return ctx.user.id
    else:
        return ctx.guild.id


@tree.command(name="oai_dm", description="DMs the specified user")
async def oai_dm(interaction: discord.Interaction, user: discord.Member, message: str):
    global brain

    if interaction.user.id != run_config.super_user:
        await interaction.response.send_message("Sorry only the super user of this bot can execute this command!")
        return

    await user.send(content=message)
    await interaction.response.send_message(f"Sent '{message}' to <@{user.id}>")


@tree.command(name="oai_dalle", description="Generates a profile picture using DALLE-2")
async def oai_dalle(interaction: discord.Interaction, prompt: str, n: int, dry: bool | None):
    global brain

    if interaction.user.id != run_config.super_user:
        await interaction.response.send_message("Sorry only the super user of this bot can execute this command!")
        return

    await interaction.response.defer()

    try:
        response = openai.Image.create(
            prompt=prompt,
            n=n,
            size="256x256"
        )

        image_url = None

        if n > 1:
            previews: list[discord.Message] = []
            embeds: list[discord.Embed] = []
            files: list[discord.File] = []

            for i in range(0, n):
                image_url = response['data'][i]['url']

                with urllib.request.urlopen(image_url) as f:
                    with open(f"{temp_folder}/temp{i}.png", "wb") as temp:
                        temp.write(f.read())

                    embed = discord.Embed(title=f"Candidate {i}")
                    embed.color = embed_color

                    discord_file = discord.File(f"{temp_folder}/temp{i}.png", filename=f"temp{i}.png")

                    embed.set_image(url=f"attachment://temp{i}.png")

                    files.append(discord_file)
                    embeds.append(embed)

            chooser: discord.Message = await interaction.channel.send(embeds=embeds, files=files, content="Please choose an image...")

            for i in range(0, n):
                await chooser.add_reaction(f"{i}⃣")

            choice: int | None = None

            while choice is None:
                try:
                    reaction, user = await bot.wait_for("reaction_add", timeout=60.0)
                    await chooser.edit(content=reaction)

                    choice = int(reaction.emoji[0])
                except Exception as e:
                    print(e)

            #await chooser.delete()

            #for message in previews:
            #    await message.delete()

            image_url = response['data'][choice]['url']
        else:
            image_url = response['data'][0]['url']

        if image_url is None:
            return

        with urllib.request.urlopen(image_url) as f:
            with open(f"{temp_folder}/temp.png", "wb") as temp:
                temp.write(f.read())

            with open(f"{temp_folder}/temp.png", "rb") as temp:
                if not dry:
                    await bot.user.edit(avatar=temp.read())

            embed = discord.Embed(title="New Profile Picture")
            embed.color = embed_color

            if dry:
                embed.description = "This was a dry run!"

            discord_file = discord.File(f"{temp_folder}/temp.png", filename="temp.png")

            embed.set_image(url="attachment://temp.png")
            await interaction.followup.send(embed=embed, file=discord_file)

            lookup = get_lookup(interaction)
            settings = brain.get_settings(lookup, interaction.guild is not None)

            if settings.channel_id != -1 and not dry:
                ann = interaction.guild.get_channel(settings.channel_id)

                if ann is not None:
                    discord_file = discord.File(f"{temp_folder}/temp.png", filename="temp.png")
                    await ann.send(file=discord_file, embed=embed)
    except discord.HTTPException as e:
        await interaction.followup.send(f"Sorry! I couldn't do that because of an exception!\n\n'{e.text}'")
        print(f"HTTP Exception! '{e.text}'")


@tree.command(name="oai_forget", description="Clears the bot's memory (useful when it starts repeating itself)")
async def oai_forget(interaction: discord.Interaction):
    guild_id = None

    if interaction.guild is not None:
        guild_id = interaction.guild.id

    brain.remember(guild_id, interaction.channel.id, interaction.user.id, "")
    await interaction.response.send_message(f"🔥🔥🔥 Cleared memory for <@{interaction.user.id}> in <#{interaction.channel.id}>! 🔥🔥🔥")


@tree.command(name="oai_reset", description="Resets the AI's settings")
async def oai_reset(interaction: discord.Interaction):
    global brain

    if interaction.guild is not None:
        if not interaction.user.guild_permissions.administrator:
            await interaction.response.send_message("This command requires admin permissions!")
            return

    lookup = get_lookup(interaction)

    embed = discord.Embed(title="Settings", description="")
    embed.color = embed_color

    embed.add_field(name="Reset", value="🔥🔥🔥 RESET 🔥🔥🔥")

    settings = AISettings()
    brain.set_settings(lookup, interaction.guild is not None, settings)

    await interaction.response.send_message(embed=embed)


@tree.command(name="oai_new_nick", description="Generates a new nickname (servers only!)")
async def oai_new_nick(interaction: discord.Interaction):
    global brain

    if interaction.guild is None:
        await interaction.response.send_message("This command only works in servers!")
        return
    else:
        if not interaction.user.guild_permissions.administrator:
            await interaction.response.send_message("This command requires admin permissions!")
            return

    lookup = get_lookup(interaction)
    settings = brain.get_settings(lookup, interaction.guild is not None)

    embed = discord.Embed(title="New Nickname", description="")
    embed.color = embed_color

    nickname = await brain.generate_nickname(interaction.guild, settings.nickname_prompt)

    embed.description = nickname
    await interaction.response.send_message(embed=embed)

    if settings.channel_id != -1:
        ann = interaction.guild.get_channel(settings.channel_id)

        if ann is not None:
            await ann.send("", embed=embed)


@tree.command(name="oai_ann", description="Tweaks the AI's status announcement (servers only)")
async def oai_ann(interaction: discord.Interaction, ann: None | discord.TextChannel):
    global brain

    if interaction.guild is None:
        await interaction.response.send_message("This command only works in servers!")
        return
    else:
        if not interaction.user.guild_permissions.administrator:
            await interaction.response.send_message("This command requires admin permissions!")
            return

    lookup = get_lookup(interaction)
    settings = brain.get_settings(lookup, interaction.guild is not None)

    embed = discord.Embed(title="Ann", description="")
    embed.color = embed_color

    if ann is None:
        embed.add_field(name="Channel", value="Disabled...")
        settings.channel_id = -1
    else:
        embed.add_field(name="Channel", value=f"<#{ann.id}>")
        settings.channel_id = ann.id

    brain.set_settings(lookup, interaction.guild is not None, settings)
    await interaction.response.send_message(embed=embed)


@tree.command(name="oai_whitelist", description="Tweaks the AI's whitelist (servers only)")
async def oai_whitelist(interaction: discord.Interaction, whitelisted: bool, channel: None | discord.TextChannel | discord.ForumChannel):
    global brain

    if interaction.guild is None:
        await interaction.response.send_message("This command only works in servers!")
        return
    else:
        if not interaction.user.guild_permissions.administrator:
            await interaction.response.send_message("This command requires admin permissions!")
            return

    lookup = get_lookup(interaction)
    settings = brain.get_settings(lookup, interaction.guild is not None)

    embed = discord.Embed(title="Whitelist", description="")
    embed.color = embed_color

    if channel is not None:
        if whitelisted:
            if channel.id not in settings.allowed_channels:
                settings.allowed_channels.append(channel.id)
        else:
            if channel.id in settings.allowed_channels:
                settings.allowed_channels.remove(channel.id)
    else:
        settings.whitelist_only = whitelisted

    embed.add_field(name="Enabled", value=settings.whitelist_only)

    channels = ""

    for channel in settings.allowed_channels:
        channels += f"<#{channel}>\n"

    embed.add_field(name="Allowed", value=channels, inline=False)

    brain.set_settings(lookup, interaction.guild is not None, settings)
    await interaction.response.send_message(embed=embed)


@tree.command(name="oai_join_vc", description="The bot joins the given channel and outputs whatever it last said to VC")
async def oai_join_vc(interaction: discord.Interaction, vc: discord.VoiceChannel):
    global voice_channel
    global voice_client

    if interaction.user.id != run_config.super_user:
        await interaction.response.send_message(f"Sorry this feature is WIP, only the creator can make the bot join vcs!")

    voice_channel = vc
    voice_client = await voice_channel.connect()

    await interaction.response.send_message(f"Joined '{vc}'")


@tree.command(name="oai_text2speech", description="Tweaks the AI's text to speech settings")
async def oai_text2speech(interaction: discord.Interaction, lang: str, tld: str):
    global tts_lang
    global tts_tld

    if interaction.user.id != run_config.super_user:
        await interaction.response.send_message(f"Sorry this feature is WIP, only the creator can tweak this!")

    tts_lang = lang
    tts_tld = tld

    await interaction.response.send_message(f"Changed lang to '{tts_lang}' and tld to '{tts_tld}'")


@tree.command(name="oai_tweak", description="Tweaks the AI's settings")
async def oai_tweak(interaction: discord.Interaction, temp: float | None, model: str | None, tokens: int | None, nick: bool | None, nick_prompt: str | None):
    global brain

    if interaction.guild is not None:
        if not interaction.user.guild_permissions.administrator:
            await interaction.response.send_message("This command requires admin permissions!")
            return

    lookup = get_lookup(interaction)
    settings = brain.get_settings(lookup, interaction.guild is not None)

    embed = discord.Embed(title="Settings", description="")
    embed.color = embed_color

    if temp is not None:
        settings.temp = temp

    if model is not None:
        settings.model = model

    if tokens is not None:
        settings.max_tokens = tokens

    if nick is not None:
        settings.nickname = nick

    if nick_prompt is not None:
        settings.nickname_prompt = nick_prompt

    embed.add_field(name="Temp", value=settings.temp, inline=False)
    embed.add_field(name="Model", value=settings.model, inline=False)
    embed.add_field(name="Max Tokens", value=settings.max_tokens, inline=False)
    embed.add_field(name="Random Nickname", value=settings.nickname, inline=False)
    embed.add_field(name="Nickname Prompt", value=settings.nickname_prompt, inline=False)

    brain.set_settings(lookup, interaction.guild is not None, settings)
    await interaction.response.send_message(embed=embed)


@tree.command(name="oai_dump", description="Dumps the brain configuration")
async def oai_dump(interaction: discord.Interaction):
    global brain

    if interaction.user.id == run_config.super_user:
        await interaction.response.send_message(file=discord.File(brain_path))
    else:
        await interaction.response.send_message("Sorry only the super user of this bot can execute this command!")


@tree.command(name="oai_prompt", description="Change / queries how prompts are composed and submitted to OpenAI")
async def oai_prompt(interaction: discord.Interaction, before: str | None, after: str | None, address: bool | None, remember: bool | None):
    global brain

    is_thread = interaction.channel.type is discord.ChannelType.public_thread

    if interaction.guild is not None:
        lacks_perms = not interaction.user.guild_permissions.administrator

        if is_thread and lacks_perms:
            lacks_perms = interaction.channel.owner.id != interaction.user.id

        if lacks_perms:
            await interaction.response.send_message("This command only works for thread OPs and admins!")
            return

    lookup = get_lookup(interaction)
    settings = brain.get_settings(lookup, interaction.guild is not None)
    composure = settings

    if is_thread:
        if interaction.channel.id not in settings.unique_settings:
            settings.unique_settings[interaction.channel.id] = ChannelSettings()

        composure = settings.unique_settings[interaction.channel.id]

    embed = discord.Embed(title="Prompt Composure", description="")
    embed.color = embed_color

    if before is not None:
        composure.before = before.strip().replace('"', '').replace("'", '')

    if after is not None:
        composure.after = after.strip().replace('"', '').replace("'", '')

    if address is not None:
        composure.address = address

    if remember is not None:
        composure.remember = remember

    brain.set_settings(lookup, interaction.guild is not None, settings)

    #predicate = "Prompt composure changed! Example of new composure\n\n"
    predicate = ""

    if composure.remember:
        predicate += "Remember, you last said 'Hola, me llamo OpenAI!'\n"

    if composure.address:
        predicate += "Address the recipient of the conversation as '{recipient}'\n"

    example = f"{predicate}{composure.before} 'Hello OpenAI!' {composure.after}"

    #if before is None and after is None:
    #    predicate = "Current prompt composure is as follows...\n\n"

    embed.description = example
    await interaction.response.send_message(embed=embed)


#
# Bootstrapper
#
def run_bot():
    global run_config

    validate_path(config_folder)
    validate_path(temp_folder)

    if os.path.exists(run_path):
        with open(run_path, "r") as f:
            raw = f.read()
            run_config = jsons.loads(raw, cls=RunConfig)
    else:
        json = jsons.dumps(run_config, jdkwargs={"indent": 4})
        validate_path(config_folder)
        with open(run_path, "w") as f:
            f.write(json)

        print("Run config was missing, please set it accordingly before next run!")
        return

    if not run_config.openai_token:
        print("Please provide an OpenAI token!")
        return

    if not run_config.discord_token:
        print("Please provide a Discord oauth token!")
        return

    # Save the config back just to create missing fields
    with open(run_path, "w") as f:
        json = jsons.dumps(run_config, jdkwargs={"indent": 4})
        f.write(json)

    openai.api_key = run_config.openai_token
    bot.run(run_config.discord_token)


run_bot()
