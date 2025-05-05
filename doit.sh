#!/bin/bash

directory=$1

if [ ! -d "$directory" ]; then
  echo "Error: Directory '$directory' not found."
  exit 1
fi

for file in "$directory"/*; do
  if [ -f "$file" ]; then
    ~/Downloads/wxWidgets-3.2.7/build-cocoa-debug/samples/swpctool/lzss d "$file" "$file"_
  fi
done