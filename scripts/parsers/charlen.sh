#/usr/bin/bash
NUM=`echo "$*" | wc -c`
let "NUM -= 1"
echo $NUM
