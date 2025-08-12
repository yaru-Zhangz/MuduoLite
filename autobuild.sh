#!/bin/bash

set -e

# 如果没有build目录，创建该目录

mkdir build && cd build
cmake .. && make -j${nproc}
