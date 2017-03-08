#!/bin/bash -e
[ -z "$3" ] && echo "Usage: $0 graph1 graph2 experiment_name"
[ -z "$3" ] && exit 1
g1=$1
g2=$2
exp_name=$3

echo -n "About to delete outputs/${exp_name}. Are you sure?"
read

rm -rf outputs/${exp_name}
mkdir -p outputs/${exp_name}/raw
mkdir -p outputs/${exp_name}/times

ulimit -t 43200
internal_scripts/shuffle_protein.sh $g1 > outputs/${exp_name}/g1.txt
(time build/tree_fast outputs/${exp_name}/g1.txt $g2 | lz4 -c6) > outputs/${exp_name}/raw/run.out.lz4 2> outputs/${exp_name}/times/run.txt

echo "Done. Now execute ./post_process.sh $g1 $g2 $exp_name"
