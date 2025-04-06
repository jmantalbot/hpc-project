#!/bin/bash

# wget https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.bz2
# tar --bzip2 -xf boost_1_85_0.tar.bz2
# cd boost_1_85_0
# ./bootstrap.sh --prefix="./" --with-libraries=mpi,serialization
# ./b2 install
# cd -

mkdir -p build
cmake -S . -B build
cmake --build build
