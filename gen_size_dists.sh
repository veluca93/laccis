#!/bin/bash

for i in `ls outputs/`
do
    echo $i
    ./size_dist.sh outputs/$i/raw | grep -B2 -A10000 Sizes > distributions/$i.raw
    ./size_dist.sh outputs/$i/recombined_filtered.txt | grep -B2 -A10000 Sizes > distributions/$i.processed
done
