#/bin/bash

# Name: charlen.sh
# Author: Ladar Levison
#
# Description: Used to run calculate the character length of the input string.

NUM=`echo "$*" | wc -c`
let "NUM -= 1"
echo $NUM
