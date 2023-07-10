import sys

import discord
from discord import app_commands
from discord.ext import tasks

import base64
import jsons
import os
import asyncio
import aiohttp
import typing
import datetime
import copy
import glob
import importlib
from importlib import util
import inspect
import sd_extension

#
# Run Settings
#
class RunConfig:
    discord_token: str = ""
    super_user: int = 363144375468949546
    preview_timeout: float = 2
    log_requests: bool = False


#
# HTTP Structures
#
class SDModelGET:
    title: str
    model_name: str


class SDSamplerGET:
    name: str


class BaseProcessingPOST:
    enable_hr: bool = False
    restore_faces: bool = False
    width: int = 512
    height: int = 512
    prompt: str = ""
    negative_prompt: str = ""
    steps: int = 24
    sampler_name: str = "DPM++ 2M Karras"
    seed: int = -1
    cfg_scale: float = 6.5
    script_name: str = ""
    script_args: list[str] = []
    n_iter: int = 1
    tiling: bool = False


class ImageProcessingPOST:
    denoising_strength: float = 0.75
    init_images: list[str] = []


#
# Global settings
#
class UserSettings:
    processing: BaseProcessingPOST = BaseProcessingPOST()
    img_processing: ImageProcessingPOST = ImageProcessingPOST()
    model: str = ""


class GlobalConfig:
    user_settings: dict[int, UserSettings] = {}
    use_whitelist: bool = False
    allowed: list[int] = []
    lifetime_images: int = 0
    word_blacklist: list[str] = []
    injections: dict[int, str] = {}


#
# Helpers
#
def inject_words(interaction: discord.Interaction, prompt: str) -> str:
    if interaction.user.id in config.injections:
        injection = config.injections[interaction.user.id]
        if injection:
            return f"{injection}. {prompt}"

    return prompt


def forbidden_words(interaction: discord.Interaction, prompt: str) -> (bool, str):
    safe_prompt = prompt.strip().lower()
    safe_prompt = safe_prompt.replace("_", "")
    safe_prompt = safe_prompt.replace("-", "")
    safe_prompt = safe_prompt.replace(" ", "")
    safe_prompt = safe_prompt.replace("0", "o")
    safe_prompt = safe_prompt.replace("3", "e")
    safe_prompt = safe_prompt.replace("7", "t")
    safe_prompt = safe_prompt.replace("1", "l")

    total = ""

    for word in config.word_blacklist:
        if word in safe_prompt:
            print(f"FORBIDDEN: {interaction.user} tried using '{word}'!")
            total += f"{word}\n"

    return len(total) > 0, total


def whitelisted(interaction: discord.Interaction) -> bool:
    if not config.use_whitelist:
        return False

    if interaction.channel.type is discord.ChannelType.public_thread:
        return interaction.channel.parent.id in config.allowed
    else:
        return interaction.channel.id in config.allowed


def validate_path(path):
    if not os.path.exists(path):
        os.mkdir(path)


def update_config():
    json = jsons.dumps(config, jdkwargs={"indent": 4})
    validate_path(config_folder)
    with open(config_path, "w") as f:
        f.write(json)


def log_request(author: discord.User, post: BaseProcessingPOST):
    if run_config.log_requests:
        validate_path(logs_folder)

        json = jsons.dumps(post, jdkwargs={"indent": 4})
        with open(logs_requests_path, "a") as f:
            f.write(f'{author.name} ({author.id}) REQUESTED @ {datetime.datetime.now()}\n\n{json}\n\n')


#
# Globals
#
intents = discord.Intents().all()
bot: discord.Client = discord.Client(intents=intents)
tree: app_commands.CommandTree = app_commands.CommandTree(bot)

config_folder = "./config"
config_path = f"{config_folder}/config.json"
run_path = f"{config_folder}/run.json"

logs_folder = "./logs"
logs_requests_path = f"{logs_folder}/requests.txt"

temp_folder = "./temp"

extensions_folder = "./extensions"

run_config: RunConfig = RunConfig()
config: GlobalConfig = GlobalConfig()

cached_models: list[SDModelGET] | None = None
cached_samplers: list[SDSamplerGET] | None = None

current_task = None
current_op: int = 0
abort_task: bool = False
skip_task: bool = False

embed_color = discord.Color.from_rgb(94, 53, 177)

extensions: list[sd_extension.ExtensionScript] = []


#
# Global functions
#
def get_banned_models() -> list[str]:
    total = []
    for extension in extensions:
        total += extension.filter_models()

    return total


async def update_status():
    await bot.change_presence(
        activity=discord.Activity(type=discord.ActivityType.watching,
                                  name=f"{config.lifetime_images} images generated..."),
        status=discord.Status.online)


async def get_cache_channel(guild: discord.Guild) -> discord.TextChannel:
    for channel in guild.channels:
        if channel.name == "sdb-bot-previews":
            return channel

    return await guild.create_text_channel("sdb-bot-previews")


async def cache_models():
    global cached_models

    if cached_models is None:
        async with aiohttp.ClientSession() as session:
            async with session.get(url="http://127.0.0.1:7860/sdapi/v1/sd-models") as req:
                if req.status == 200:
                    cached_models = []
                    js = await req.json()

                    for elem in js:
                        model = jsons.load(elem, cls=SDModelGET)
                        cached_models.append(model)

                        print(f"DIFFUSION: Found model '{model.title}'")


async def cache_samplers():
    global cached_samplers

    if cached_samplers is None:
        async with aiohttp.ClientSession() as session:
            async with session.get(url="http://127.0.0.1:7860/sdapi/v1/samplers") as req:
                cached_samplers = []
                if req.status == 200:
                    js = await req.json()

                    for elem in js:
                        sampler = jsons.load(elem, cls=SDSamplerGET)
                        cached_samplers.append(sampler)

                        print(f"DIFFUSION: Found sampler '{sampler.name}'")


async def switch_model(model: str):
    global cached_models
    await cache_models()

    # First verify that the model actually exists
    exists = False
    for cached in cached_models:
        if cached.title == model:
            exists = True
            break

    if not exists:
        print(f"DIFFUSION: Model '{model}' doesn't exist!")
        return

    async with aiohttp.ClientSession() as session:
        async with session.get(url="http://127.0.0.1:7860/sdapi/v1/options") as req:
            if req.status == 200:
                js = await req.json()

                if js['sd_model_checkpoint'] != model:
                    js['sd_model_checkpoint'] = model

                    async with session.post(url="http://127.0.0.1:7860/sdapi/v1/options", json=js) as ps:
                        await ps.json()
                        print(f"DIFFUSION: Successfully changed model to {model}!")


def super_user_check():
    def predicate(interaction: discord.Interaction) -> bool:
        return interaction.user.id == run_config.super_user

    return app_commands.check(predicate)


#
# Bot callbacks
#
@bot.event
async def on_ready():
    global bot
    global config

    if os.path.exists(config_path):
        with open(config_path, "r") as f:
            raw = f.read()
            config = jsons.loads(raw, cls=GlobalConfig)

    print(f"CLIENT: Logged in as bot user {bot.user.name}")

    await tree.sync()
    await update_status()


@bot.event
async def on_reaction_add(reaction: discord.Reaction, user: discord.Member | discord.User):
    global abort_task
    global skip_task

    if reaction.emoji == "🛑" and (user.id == current_op or user.id == run_config.super_user):
        abort_task = True

    if reaction.emoji == "⏩" and (user.id == current_op or user.id == run_config.super_user):
        skip_task = True


#
# Commands
#
@tree.command(name="diffuse_inject", description="Changes a users pre-prompt injection")
@super_user_check()
@app_commands.default_permissions(administrator=True)
async def diffuse_inject(interaction: discord.Interaction, user: discord.Member, injected: str = ""):
    global config

    if interaction.user.id != run_config.super_user:
        await interaction.response.send_message(f"Sorry only the super user is allowed to execute this!")
        return

    await interaction.response.defer(ephemeral=True)

    config.injections[user.id] = injected

    update_config()
    await interaction.followup.send(f"Injecting '{injected}' into <@{user.id}>'s prompts!")


@tree.command(name="diffuse_banlist", description="Adds / removes a word from the banlist, separate words w/ semicolons!")
@super_user_check()
@app_commands.default_permissions(administrator=True)
async def diffuse_banlist(interaction: discord.Interaction, blocked: bool, words: str):
    global config

    if interaction.user.id != run_config.super_user:
        await interaction.response.send_message(f"Sorry only the super user is allowed to execute this!")
        return

    await interaction.response.defer(ephemeral=True)

    word_list = words.split(';')

    for word in word_list:
        actual = word.lower().strip()

        if not actual:
            continue

        if actual in config.word_blacklist and not blocked:
            config.word_blacklist.remove(actual)
            print(f"BANLIST: Allowed {actual}")

        if actual not in config.word_blacklist and blocked:
            config.word_blacklist.append(actual)
            print(f"BANLIST: Blocked {actual}")

    action = "Blocked"

    if not blocked:
        action = "Allowed"

    update_config()
    await interaction.followup.send(f"{action} {len(word_list)} words!")


@tree.command(name="diffuse_purge", description="Purges messages from the current channel")
@super_user_check()
@app_commands.default_permissions(administrator=True)
async def diffuse_purge(interaction: discord.Interaction, amount: int):
    if interaction.user.id != run_config.super_user:
        await interaction.response.send_message(f"Sorry only the super user is allowed to execute this!")
        return

    await interaction.response.defer(ephemeral=True)

    resp = await interaction.channel.purge(limit=amount)
    await interaction.followup.send(f"Purged {len(resp)} messages 🔥🔥🔥")


@tree.command(name="diffuse_whitelist", description="Changes if the bot uses a whitelist")
@super_user_check()
@app_commands.default_permissions(administrator=True)
async def diffuse_whitelist(interaction: discord.Interaction, allowed: bool,
                            channel: discord.ForumChannel | discord.TextChannel | None):
    global config

    if interaction.user.id != run_config.super_user:
        await interaction.response.send_message(f"Sorry only the super user is allowed to execute this!")
        return

    if channel is not None:
        if allowed and interaction.user.id not in config.allowed:
            config.allowed.append(channel.id)
            await interaction.response.send_message(f"Allowed <#{channel.id}>")

        if not allowed and interaction.user.id in config.allowed:
            config.allowed.remove(channel.id)
            await interaction.response.send_message(f"Removed <#{channel.id}>")
    else:
        config.use_whitelist = allowed
        await interaction.response.send_message(f"Use whitelist = {allowed}")

    update_config()


@tree.command(name="diffuse_png_info", description="Returns PNG info (if it is found)")
async def diffuse_png_info(interaction: discord.Interaction, file: discord.Attachment):
    if not whitelisted(interaction):
        return

    await interaction.response.defer()

    async with aiohttp.ClientSession() as session:
        with open(f"{temp_folder}/png_info.png", "wb") as d:
            await file.save(d)

        with open(f"{temp_folder}/png_info.png", "rb") as d:
            data = d.read()

        encoded = base64.b64encode(data)

        payload = {
            "image": encoded.decode()
        }

        async with session.post(url="http://127.0.0.1:7860/sdapi/v1/png-info", json=payload) as req:
            if req.status == 200:
                js = await req.json()

                embed = discord.Embed()
                embed.colour = embed_color
                embed.title = "PNG Info"

                raw_params: str = js['items']['parameters']

                tokens = raw_params.replace('\n', ',').split(",")

                # Prompt is always token zero
                prompt = tokens[0]
                tokens.pop(0)

                embed.add_field(name="Prompt", value=prompt, inline=False)

                # Everything else comes after
                params = {}
                for token in tokens:
                    # Get the param name
                    split = token.split(":", 1)
                    name = split[0].strip()
                    value = split[1].removeprefix(":").strip()

                    embed.add_field(name=name, value=value, inline=False)
                    params[name.lower()] = value

                # Guess if this is img2img
                cmd = "/diffuse_txt2img"

                is_img2img = 'mask blur' in params

                if is_img2img:
                    cmd = "/diffuse_img2img image:"

                foot = f"{cmd} prompt:{prompt}"

                if 'negative prompt' in params:
                    foot += f" negative_prompt:{params['negative prompt']}"

                if 'seed' in params:
                    foot += f" seed:{params['seed']}"

                if 'size' in params:
                    split = params['size'].split('x')

                    width = int(split[0])
                    height = int(split[1])

                    foot += f" width:{width} height:{height}"

                if 'denoising strength' in params and is_img2img:
                    foot += f" denoising:{params['denoising strength']}"

                embed.set_footer(text=foot)
                await interaction.followup.send(embed=embed)
            else:
                print(f"ERROR: Unknown status '{req.status}'")


@tree.command(name="diffuse_tweak", description="Tweaks unique user settings for Stable Diffusion")
async def diffuse_tweak(interaction: discord.Interaction,
                        model: str | None,
                        steps: int | None,
                        sampler: str | None,
                        cfg_scale: float | None):
    global config

    if not whitelisted(interaction):
        return

    # Check if the user has an entry
    settings = UserSettings()
    if interaction.user.id not in config.user_settings:
        new_settings = UserSettings()
        new_settings.model = cached_models[0].title

        config.user_settings[interaction.user.id] = new_settings
    else:
        settings = config.user_settings[interaction.user.id]

    if model is not None:
        settings.model = model

    if steps is not None:
        if interaction.user.id == run_config.super_user:
            settings.processing.steps = steps
        else:
            settings.processing.steps = max(min(steps, 90), 0)

    if sampler is not None:
        settings.processing.sampler_name = sampler

    if cfg_scale is not None:
        settings.processing.cfg_scale = cfg_scale

    embed = discord.Embed()
    embed.colour = embed_color
    embed.title = "Settings"

    embed.add_field(name="Model", value=settings.model, inline=False)
    embed.add_field(name="Sampler", value=settings.processing.sampler_name, inline=False)
    embed.add_field(name="Steps", value=settings.processing.steps, inline=False)
    embed.add_field(name="CFG Scale", value=settings.processing.cfg_scale, inline=False)

    config.user_settings[interaction.user.id] = settings
    await interaction.response.send_message(embed=embed)

    update_config()


async def post_for_image(interaction: discord.Interaction, settings: UserSettings, prompt: str, img2img: bool):
    global config

    # 10 minute timeout, better hope your image doesn't process for that damn long
    timeout = aiohttp.ClientTimeout(total=600)

    async with aiohttp.ClientSession(timeout=timeout) as session:
        old = settings.processing.prompt
        settings.processing.prompt = prompt

        payload = jsons.dump(settings.processing)

        if img2img:
            payload = payload | jsons.dump(settings.img_processing)

        settings.processing.prompt = old

        endpoint = "txt2img"

        if img2img:
            endpoint = "img2img"

        async with session.post(url=f"http://127.0.0.1:7860/sdapi/v1/{endpoint}", json=payload) as req:
            if req.status == 200:
                js = await req.json()

                info = jsons.loads(js['info'])
                seed = info['seed']

                embed = discord.Embed(title="Result")
                embed.colour = embed_color

                embed.set_author(name="Stable Diffusion", icon_url=bot.user.avatar.url)

                embed.title = settings.processing.prompt

                embed.add_field(name="Sampler", value=settings.processing.sampler_name)
                embed.add_field(name="Steps", value=settings.processing.steps)
                embed.add_field(name="CFG Scale", value=settings.processing.cfg_scale)
                embed.add_field(name="Seed", value=seed)
                embed.add_field(name="Model", value=settings.model)

                if img2img:
                    embed.add_field(name="Denoising", value=settings.processing.denoising_strength)

                if settings.processing.negative_prompt:
                    embed.description = f"Negative Prompt: {settings.processing.negative_prompt}"

                files = []

                num = 0

                local_path = f"{temp_folder}/{interaction.user.id}"
                validate_path(local_path)

                for key in js['images']:
                    out_path = f"{local_path}/{seed}-{num}.png"

                    with open(out_path, "wb") as d:
                        data = base64.b64decode(key)
                        d.write(data)

                    discord_file = discord.File(out_path, filename=f"{seed}-{num}.png")
                    files.append(discord_file)

                    num += 1
                    config.lifetime_images += 1

                settings.processing.seed = seed
                settings.img_processing.init_images = []

                try:
                    await interaction.followup.send(embed=embed, files=files)
                except discord.HTTPException as e:
                    await interaction.followup.send(f"HTTP ERROR {e.code}: '{e.text}'")

                await update_status()


async def diffuse_process(interaction: discord.Interaction,
                          prompt: str,
                          negative_prompt: str = "",
                          seed: int = -1,
                          rem_bg: bool = False,
                          tiling: bool = False,
                          count: int = 1,
                          width: int = 512,
                          height: int = 512,
                          images: list[str] = [],
                          denoise: float = 0.75):
    global current_task
    global current_op
    global abort_task
    global skip_task

    if not whitelisted(interaction):
        return

    await interaction.response.defer()

    # Check if the user has an entry
    settings = UserSettings()
    if interaction.user.id not in config.user_settings:
        new_settings = UserSettings()
        new_settings.model = cached_models[0].title

        config.user_settings[interaction.user.id] = new_settings
    else:
        settings = config.user_settings[interaction.user.id]

    await switch_model(settings.model)

    if rem_bg:
        settings.processing.script_name = "abg remover"
        settings.processing.script_args = [False, False]
    else:
        settings.processing.script_name = ""
        settings.processing.script_args = []

    actual_prompt = inject_words(interaction, prompt)

    if actual_prompt != prompt:
        print(f"DIFFUSION: Spoofing prompt as '{actual_prompt}'")

    settings.processing.prompt = prompt
    settings.processing.negative_prompt = negative_prompt
    settings.processing.seed = seed
    settings.processing.width = min(512, max(64, width))
    settings.processing.height = min(512, max(64, height))
    settings.processing.n_iter = min(4, max(1, count))
    settings.processing.tiling = tiling

    if rem_bg:
        settings.processing.n_iter = 1

    is_img2img = len(images) > 0

    if is_img2img:
        settings.img_processing.init_images = images
        settings.img_processing.denoising_strength = denoise

    settings.processing.prompt = actual_prompt
    log_request(interaction.user, settings.processing)
    settings.processing.prompt = prompt

    forbidden, blocked = forbidden_words(interaction, prompt)

    super_mention = f"<@{run_config.super_user}>"

    if forbidden:
        embed = discord.Embed()
        embed.colour = embed_color
        embed.title = f"Incident Report"
        embed.description = f"Prompt: {prompt}"
        embed.add_field(name="Blocked tokens", value=blocked)

        await interaction.followup.send(content=super_mention, embed=embed)
        return

    forbidden, blocked = forbidden_words(interaction, prompt)

    if forbidden:
        embed = discord.Embed()
        embed.colour = embed_color
        embed.title = f"Incident Report"
        embed.description = f"Prompt: {negative_prompt}"
        embed.add_field(name="Blocked tokens", value=blocked)

        await interaction.followup.send(content=super_mention, embed=embed)
        return

    update_config()

    while current_task is not None:
        # print("Waiting for other task!")
        await asyncio.wait([current_task], timeout=0.5)

    # Blocks if there isn't a task open
    async with aiohttp.ClientSession() as session:
        blocked = True
        while blocked:
            await asyncio.sleep(0.5)
            async with session.get(url="http://127.0.0.1:7860/sdapi/v1/progress?skip_current_image=false") as req:
                if req.status == 200:
                    js = await req.json()
                    blocked = js['state']['job_count'] > 0 or js['current_image'] is not None

            # print("WebUI is blocking!")

    current_task = asyncio.create_task(post_for_image(interaction, settings, actual_prompt, is_img2img))
    current_op = interaction.user.id

    async with aiohttp.ClientSession() as session:
        cache = await get_cache_channel(interaction.guild)

        out_path = f"{temp_folder}/temp.png"

        preview: discord.Message | None = None
        embed = discord.Embed()
        embed.colour = embed_color
        embed.title = f"Preview for '{interaction.user.name}'"
        embed.description = f"Prompt: {prompt}"

        embed.set_author(name="Stable Diffusion", icon_url=bot.user.avatar.url)

        if negative_prompt:
            embed.description += f"\n\nNegative Prompt: {negative_prompt}"

        while True:
            skipped = False

            if abort_task:
                async with session.post(url="http://127.0.0.1:7860/sdapi/v1/interrupt") as req:
                    await req.json()

                abort_task = False
                skipped = True

                if preview is not None:
                    await preview.delete()
                    preview = None

            if skip_task:
                async with session.post(url="http://127.0.0.1:7860/sdapi/v1/skip") as req:
                    await req.json()

                skip_task = False
                skipped = True

                if preview is not None:
                    await preview.delete()
                    preview = None

            done, pending = await asyncio.wait([current_task], timeout=run_config.preview_timeout)

            if len(done) > 0:
                current_task = None

                if preview is not None:
                    await preview.delete()

                update_config()
                break
            elif not skipped:
                # Check the progress of our current prompt
                async with session.get(url="http://127.0.0.1:7860/sdapi/v1/progress?skip_current_image=false") as req:
                    if req.status == 200:
                        js = await req.json()

                        if js is not None:
                            current = js['state']['sampling_step']
                            eta = js['eta_relative']
                            eta = round(eta, ndigits=2)

                            percent = js['progress']
                            percent = round(percent * 100)

                            foot = f"{current}/{settings.processing.steps} Steps; {percent}%; ETA {eta}s"

                            if js['state']['job_count'] > 1:
                                job_no = js['state']['job_no'] + 1
                                job_count = js['state']['job_count']

                                foot += f"; Job ({job_no}/{job_count})"

                            embed.set_footer(text=foot)

                            if js['current_image'] is not None:
                                with open(out_path, "wb") as d:
                                    data = base64.b64decode(js['current_image'])
                                    d.write(data)

                                discord_file = discord.File(out_path, filename="preview.png")
                                msg = await cache.send(file=discord_file)
                                pic = msg.attachments[0].url

                                embed.set_image(url=pic)

                                if preview is None:
                                    preview = await interaction.channel.send(embed=embed)
                                    await preview.add_reaction("🛑")
                                    await preview.add_reaction("⏩")
                                else:
                                    await preview.edit(embed=embed)


@tree.command(name="diffuse_reset", description="Reset settings for Stable Diffusion")
async def diffuse_reset(interaction: discord.Interaction):
    await cache_models()

    settings = UserSettings()
    settings.model = cached_models[0].title

    config.user_settings[interaction.user.id] = settings

    update_config()

    await interaction.response.send_message("🔥🔥🔥 Reset settings!!! 🔥🔥🔥")


@tree.command(name="diffuse_txt2img", description="Generates an image from a text prompt using Stable Diffusion")
async def diffuse_txt2img(interaction: discord.Interaction,
                          prompt: str,
                          negative_prompt: str = "",
                          seed: int = -1,
                          rem_bg: bool = False,
                          count: int = 1,
                          width: int = 512,
                          height: int = 512,
                          tiling: bool = False):
    await diffuse_process(interaction, prompt, negative_prompt, seed, rem_bg, tiling, count, width, height)


@tree.command(name="diffuse_img2img",
              description="Generates an image from a text prompt and reference image using Stable Diffusion")
async def diffuse_img2img(interaction: discord.Interaction,
                          image: discord.Attachment,
                          prompt: str,
                          negative_prompt: str = "",
                          seed: int = -1,
                          rem_bg: bool = False,
                          width: int = 512,
                          height: int = 512,
                          denoising: float = 0.75,
                          tiling: bool = False,
                          count: int = 1):

    with open(f"{temp_folder}/img2img.png", "wb") as d:
        await image.save(d)

    with open(f"{temp_folder}/img2img.png", "rb") as d:
        data = d.read()

    encoded = base64.b64encode(data)
    str_img = encoded.decode()

    denoise = min(1, max(0, denoising))

    await diffuse_process(interaction, prompt, negative_prompt, seed, rem_bg, tiling, count, width, height, [str_img],
                          denoise)


@diffuse_tweak.autocomplete('model')
async def diffuse_model_autocomplete(interaction: discord.Interaction, current: str) -> [app_commands.Choice[str]]:
    global cached_models
    await cache_models()

    banned_models = get_banned_models()
    model_list = []
    for model in cached_models:
        if model.model_name in banned_models or model.title in banned_models:
            continue

        model_list.append(app_commands.Choice(name=model.model_name, value=model.title))

    return model_list


@diffuse_tweak.autocomplete('sampler')
async def diffuse_samplers_autocomplete(interaction: discord.Interaction, current: str) -> [app_commands.Choice[str]]:
    global cached_samplers
    await cache_samplers()

    sampler_list = []
    for sampler in cached_samplers:
        sampler_list.append(app_commands.Choice(name=sampler.name, value=sampler.name))

    return sampler_list


@diffuse_txt2img.autocomplete('prompt')
@diffuse_img2img.autocomplete('prompt')
async def diffuse_prompt_autocomplete(interaction: discord.Interaction, current: str) -> [app_commands.Choice[str]]:
    if interaction.user.id in config.user_settings:
        last = config.user_settings[interaction.user.id].processing.prompt
        return [app_commands.Choice(name=last, value=last)]
    else:
        return []


@diffuse_txt2img.autocomplete('negative_prompt')
@diffuse_img2img.autocomplete('negative_prompt')
async def diffuse_negative_autocomplete(interaction: discord.Interaction, current: str) -> [app_commands.Choice[str]]:
    if interaction.user.id in config.user_settings:
        last = config.user_settings[interaction.user.id].processing.negative_prompt
        return [app_commands.Choice(name=last, value=last)]
    else:
        return []


@diffuse_txt2img.autocomplete('seed')
@diffuse_img2img.autocomplete('seed')
async def diffuse_seed_autocomplete(interaction: discord.Interaction, current: str) -> [app_commands.Choice[int]]:
    if interaction.user.id in config.user_settings:
        last = config.user_settings[interaction.user.id].processing.seed
        return [app_commands.Choice(name=f"{last}", value=last)]
    else:
        return []


#
# Bootstrap
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

    if not run_config.discord_token:
        print("Please provide a Discord oauth token!")
        return

    # Save the config back just to create missing fields
    with open(run_path, "w") as f:
        json = jsons.dumps(run_config, jdkwargs={"indent": 4})
        f.write(json)

    # Try loading all found extensions
    for ext in glob.glob(f"{extensions_folder}/*/module.py"):
        name = ext.replace(extensions_folder, "").removeprefix("/").removeprefix("\\")
        spec = importlib.util.spec_from_file_location(name, ext)
        mod = importlib.util.module_from_spec(spec)

        sys.modules[name] = mod
        spec.loader.exec_module(mod)

        print("--== LOADED MODULE ==--")
        print(f"Name = {mod.extension_script.get_name()}")
        print(f"Author = {mod.extension_script.get_author()}")
        print("--===================--")

        print("--== MODULE BLOCKS ==--")
        print(f"Blocked Models = {mod.extension_script.filter_models()}")
        print("--===================--")

        extensions.append(mod.extension_script)

    bot.run(run_config.discord_token)


run_bot()
