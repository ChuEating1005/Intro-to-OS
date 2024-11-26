#!/bin/bash

# clear
echo "Compile main.cpp ..."
g++ -o main hw5_111550093.cpp

echo -e "\nRun testcase : testcase.txt ...\n--------------------------------------------------------------------------------"
./main testcase.txt
echo "--------------------------------------------------------------------------------"
