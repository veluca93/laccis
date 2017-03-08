#!/usr/bin/python3

import sys
import math

lines = [tuple(map(int, x.split())) for x in sys.stdin.readlines()]
h = 0
pos = len(lines)-1
while pos > -1 and h < lines[pos][0]:
    h += lines[pos][1]
    pos -= 1

if pos == -1:
    print(h)
else:
    print(lines[pos][0])
