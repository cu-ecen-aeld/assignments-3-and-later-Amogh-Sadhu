#!/bin/sh

if [ $# -lt 2 ]; then
    echo "Error: Two arguments required. Usage: ./writer.sh <filepath> <string>"
    exit 1
fi

writefile=$1
writestr=$2

dirpath=$(dirname "$writefile")
mkdir -p "$dirpath"

echo "$writestr" > "$writefile"

if [ $? -ne 0 ]; then
    echo "Error: The file could not be created."
    exit 1
fi
