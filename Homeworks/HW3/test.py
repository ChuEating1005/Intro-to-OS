import argparse
from random import randint

# Add argument parser
parser = argparse.ArgumentParser(description='Generate random numbers and write to a file.')
parser.add_argument('-N', type=int, default=100000, help='Number of random numbers to generate')
args = parser.parse_args()

N = args.N
with open('input.txt', 'w') as f:
    f.write(f'{N}\n')
    for i in range(N):
        f.write(f'{randint(-2147483648, 2147483647)} ')
