#!/bin/bash

cmake -DCMAKE_INSTALL_PREFIX=$PWD/out/install/x64-clang-linux-release -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=Release -S $PWD -B $PWD/out/build/x64-clang-linux-release -G Ninja 

cmake --build $PWD/out/build/x64-clang-linux-release --target main

sudo $PWD/out/build/x64-clang-linux-release/main
