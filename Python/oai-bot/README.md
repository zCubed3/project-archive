# OAI-Bot - Discord.py and OpenAI powered chatbot
[![forthebadge](https://forthebadge.com/images/badges/made-with-python.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/makes-people-smile.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/60-percent-of-the-time-works-every-time.svg)](https://forthebadge.com)

### Python 3.10 Required! Anything newer or older doesn't work!

### Usage Instructions
  1) Create a Discord Application, and allocate an OAuth bot token for it
     1) Also invite the bot to the servers you want it in (you have to be admin!) 
  2) Run "pip install -r requirements.txt"
     1) NOTE: On Linux you might have to install `build-essentials`!
  3) Run the bot at least once to generate config files
  4) Edit `config/run.json` and adjust the following
     1) Set `openai_token` to your OpenAI API token
     2) Set `discord_token` to your Discord OAuth token
     3) Set `username` if you want to change the username manually
  5) Run `python ./main.py`
     1) If you're using docker pay attention!
        1) Run `./build.sh` for a first run
        2) Run `./run.sh` to start the bot after subsequent runs
  6) If everything goes to plan, it should run just fine!
