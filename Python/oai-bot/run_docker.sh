#!/bin/bash

build_image() {
	if docker ps -a | grep -q "openai-bot"; then
		echo "Attempting to stop and remove existing container..."
    	docker stop openai-bot
    	docker container rm openai-bot
	fi

	if [ "$(docker images -q zcubed/openai-bot:latest 2> /dev/null)" = "" ]; then
		echo "Existing image found, removing it..."
	    docker image rm zcubed/openai-bot:latest
	fi

	docker build --tag zcubed/openai-bot .
}

run_bot() {
	if docker ps -a | grep -q "openai-bot"; then
    	echo "Bot container already exists, stopping the container!"
    	docker stop openai-bot
    fi

    echo "Pulling..."
    git pull

    manifest=$(<version.txt)

    last_version=0

    if test -f ".last_version"; then
    	last_version=$(<.last_version)
    else
    	echo "Version manifest was missing! Forcing rebuild due to unknown version!"
    fi

	if [ "$manifest" -gt "$last_version" ]; then
		echo "Out of date! Rebuilding image!"
		build_image
	else
		echo "Up to date! No need to rebuild :)"
	fi

	if docker ps -a | grep -q "openai-bot"; then
		echo "Starting existing container..."
		docker start openai-bot
	else
		echo "Creating new container..."
		docker run --restart=always --network=host -dit -v "$(pwd):/bot" --name "openai-bot" zcubed/openai-bot:latest
	fi

    echo "$manifest" > ".last_version"
}

run_bot