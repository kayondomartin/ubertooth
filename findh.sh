#!/bin/bash
clear
printf "\n"
echo kjyoon find "$2" in "$1".
if [ -z "$1" ]
then
  find . -name "*.h" | xargs grep $2 -n
else
  find $1 -name "*.h" | xargs grep $2 -n
fi
