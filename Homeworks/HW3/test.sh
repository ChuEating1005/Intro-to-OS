#!/bin/bash

python3 test.py
echo "Testcase generated"
g++ -pthread hw3_111550093.cpp -o hw3.out
echo "Compiled"
echo ""
output=$(./hw3.out | tee /dev/tty)
echo "$output" > performance.txt
python3 plot.py
