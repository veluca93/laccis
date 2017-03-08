#!/usr/bin/python3

import sys
import math

lines = [tuple(map(int, x.split())) for x in sys.stdin.readlines()]
vals = list(map(lambda x: 2**x[0]*x[1], lines))
counts = list(map(lambda x: x[1], lines))
mean = math.log2(sum(vals)/sum(counts))
print(mean)
