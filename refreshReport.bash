#!/bin/bash



rm -f data/*
./getFBAds.js || exit 1

./refreshIndex.bash
