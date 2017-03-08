#!/bin/bash
head -n2 $1
tail -n +3 $1 | shuf
