#!/bin/bash

python3 test.py -N 100000
echo "Testcase generated"
g++ -pthread hw3_111550093.cpp -o hw3.out
echo "Compiled"
echo "Running..."
output=$(./hw3.out)
echo -e "\nTime: "
echo "$output"
echo -e "\nTestcase: "
python3 result.py
echo "$output" > performance.txt
echo -e "\nTime curve:"
python3 plot.py
