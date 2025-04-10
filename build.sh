#!/bin/bash

bash install_boost.sh

mkdir -p build
cmake -S . -B build
cmake --build build
