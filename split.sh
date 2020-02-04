#!/bin/sh
output=$(cat "-")
echo "$output" | grep "youtube" | xargs ./youtube.py
echo "$output" | grep -v "youtube" > ~/blogs.txt
