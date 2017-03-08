#!/bin/bash -e

PATH_TO_LIBLZ4=/samba/libs/lz4/lib

CPP=g++
CFLAGS="-O3 -Wall -march=native -ggdb -std=c++14 -I src -Wno-unused-result -Wno-unused-local-typedefs -pthread"
LDFLAGS="-lpthread -lboost_thread -lboost_system"

compile() {
    file=$1
    [ src/$file.cpp -nt build/$file ] || return 0
    echo Compiling $file
    shift
    $CPP $CFLAGS -o build/$file src/$file.cpp $LDFLAGS $*
}

mkdir -p build

compile sort_iso $PATH_TO_LIBLZ4/liblz4.a -I $PATH_TO_LIBLZ4
compile cuckoo_implicit
compile combiner
compile diversificator
compile incremental_wc
compile tree_fast
compile tree_cover
