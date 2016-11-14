#!/bin/bash
SNUMAP_ROOT=$PWD
export SNUMAP_ROOT

export PATH=$PATH:$SNUMAP_ROOT/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SNUMAP_ROOT/lib

export C_INCLUDE_PATH=$C_INCLUDE_PATH:$SNUMAP_ROOT/inc
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$SNUMAP_ROOT/inc