#!/bin/bash -e

tl=43200

[ -z "$4" ] && echo "Usage: $0 graph1 graph2 number_of_spt experiment_name"
[ -z "$4" ] && echo && echo "If the number of spanning trees is 0, generate a cover of the graph"
[ -z "$4" ] && exit 1
g1=$1
g2=$2
n_spt=$3
exp_name=$4

echo -n "About to delete outputs/${exp_name}. Are you sure?"
read

rm -rf outputs/${exp_name}
mkdir -p outputs/${exp_name}/raw
mkdir -p outputs/${exp_name}/times

kill_jobs() {
	jobs -p | xargs kill
	exit 1
}

trap kill_jobs SIGINT

if [ ! $n_spt -gt 0 ]
then
    n_spt=$(build/tree_cover outputs/${exp_name}/g1 < $g1 2> /dev/null)
else
    for i in `seq 1 $n_spt`
    do
        internal_scripts/shuffle_protein.sh $g1 > outputs/${exp_name}/g1_${i}.txt
    done
fi

let time_limit=$tl/$n_spt
ulimit -t $time_limit
for i in `seq 1 $n_spt`
do
    (time build/tree_fast outputs/${exp_name}/g1_${i}.txt $g2 | lz4 -c7) > outputs/${exp_name}/raw/${i}.out.lz4 2> outputs/${exp_name}/times/${i}.txt &
done

wait
echo "Done. Now execute ./post_process.sh $g1 $g2 $exp_name"
