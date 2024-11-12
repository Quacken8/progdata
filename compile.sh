#!/bin/bash

FILES=$(ls sourcefiles/*)

for file in $FILES
do
	gcc -O2 -o sourcefiles/${file}.out sourcefiles/${file}
done