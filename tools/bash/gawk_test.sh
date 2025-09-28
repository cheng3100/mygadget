#! /bin/bash

info=$(ps -e | gawk '$4 ~ /firefox/{print $1}') 
echo $info
for i in $info 
do
    echo "the pid is:" $i
    kill -9 $i
done
