#!/bin/bash
OMP_PROFILER_ROOT=$PWD
export OMP_PROFILER_ROOT

SNUMAP_ROOT=$PWD
export SNUMAP_ROOT

export PATH=$PATH:$SNUMAP_ROOT/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SNUMAP_ROOT/lib

export C_INCLUDE_PATH=$C_INCLUDE_PATH:$SNUMAP_ROOT/inc
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$SNUMAP_ROOT/inc

cd $SNUMAP_ROOT/src/kernel-module/
make clean
./uninstall.sh

cd $SNUMAP_ROOT/src/snumap-interface/dynamic_c/
make clean

cd $SNUMAP_ROOT/src/snumap-interface/static_c/
make clean

cd $SNUMAP_ROOT/src/snumap-main/
make clean

cd $SNUMAP_ROOT/src/snumap-numa/
make clean
