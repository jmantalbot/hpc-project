#!/bin/bash

bash load_modules.sh
bash install_boost.sh

mkdir -p build
cmake -S . -B build
cmake --build build
