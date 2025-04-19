#!/bin/bash

if [ -d boost_1_88_0 ]; then
  echo "Found boost_1_88_0, skipping download and build."
else
  echo "Installing boost..."
  wget https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.bz2
  tar --bzip2 -xvf boost_1_88_0.tar.bz2
  cd boost_1_88_0
  ./bootstrap.sh --prefix="./" --with-libraries=mpi,serialization
  echo "using mpi ;" >> project-config.jam
  ./b2 install
  cd -
  echo "Installed boost."
fi