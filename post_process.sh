#!/bin/bash -e
[ -z "$3" ] && echo "Usage: $0 graph1 graph2 experiment_name [retain_sz [retain_perc]]"
[ -z "$3" ] && exit 1
g1=$1
g2=$2
exp_name=$3

echo -n "About to delete outputs/${exp_name}/sorted and related files. Are you sure?"
read

rm -rf outputs/${exp_name}/sorted/
mkdir -p outputs/${exp_name}/sorted/

list_files() {
    for i in `ls $1 | sort -nr`
    do
        echo $1/$i
    done
}

export PATH=/samba/libs/lz4/programs/:$PATH

echo "Sorting..."
cat `list_files outputs/${exp_name}/raw` | lz4 -dc | (time build/sort_iso outputs/${exp_name}/sorted/ 6) 2> outputs/${exp_name}/times/sort

echo "Filtering..."
cat `list_files outputs/${exp_name}/sorted` | lz4 -dc | (time build/diversificator 6 $4 $5) > outputs/${exp_name}/filtered.txt 2> outputs/${exp_name}/times/filter_first

echo "Recombining..."
(time build/combiner $g1 $g2) < outputs/${exp_name}/filtered.txt > outputs/${exp_name}/recombined.txt 2> outputs/${exp_name}/times/recombine

echo "Filtering again..."
(time build/diversificator 1 $4 $5) < outputs/${exp_name}/recombined.txt > outputs/${exp_name}/recombined_filtered.txt 2> outputs/${exp_name}/times/filter_second
