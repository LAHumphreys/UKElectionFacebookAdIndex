#!/bin/bash

source API_TOKEN.sh || exit 1

if [[ "$FB_API_TOKEN" == "" ]]; then
    echo "ERROR: The Facebook API token must be defined in FB_API_TOKEN"
    exit 1
fi

rm -f data/*
mkdir -p data
./getFBAds.js || exit 1

./refreshIndex.bash $@
