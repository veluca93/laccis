# LACCIS-finder
A tool to find LACCIS in two graphs.
Basic usage is as follows:

- Please make sure that you have `lz4` available in your path
  and `liblz4.a` installed in your system. Set the variable
  `PATH_TO_LIBLZ4` in `compile_everything.sh` to the correct
  folder that contains that library.

- Set the total time limit in `multiple_spt.sh`

- Run


    ./compile_everything.sh
    ./multiple_spt.sh graph1 graph2 number_of_spt experiment_name
    ./post_process.sh graph1 graph2 experiment_name

where the graphs should be in the following format (N = number of nodes, M = number of edges):

    N M
    N integers: the labels of the nodes, in order
    M pairs of integers: the endpoints of an edge

Some extra details are available by running `multiple_spt.sh` or `post_process.sh` without arguments.
