#!/bin/bash

cmake -DCMAKE_INSTALL_PREFIX=$PWD/out/install/x64-clang-linux-debug -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=Debug -S $PWD -B $PWD/out/build/x64-clang-linux-debug -G Ninja

cmake --build $PWD/out/build/x64-clang-linux-debug --target main

$PWD/out/build/x64-clang-linux-debug/main
