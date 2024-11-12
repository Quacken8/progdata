#!/bin/bash

FILES=$(ls */*.c)
for file in $FILES
do
	outName=$(echo "$file" | cut -d'.' -f1)
	gcc -O2 -o "$outName".out "$file"
done