#!/bin/bash

echo "Compile main.c ..."
gcc main.c -o main 

echo "Compile hw4_111550093.c ..."
gcc -shared -fPIC hw4_111550093.c -o multilevelBF.so

echo -e "Done\n\n---------------------------------\nBest fit :"
LD_PRELOAD=./multilevelBF.so ./main

echo "---------------------------------"