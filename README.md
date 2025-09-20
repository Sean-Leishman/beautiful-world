# Instructions to Run

First off attempt to run `./cmake.bash`. If this does not work due to not finding `lomp` install  via `sudo apt-get install libomp-dev`. 

Clang++14 is the preferred compiler to run this. If this is not acceptable it may be more suitable to run `cmake_debug.bash` in order to build and run the debug version without the optimisations. 

The `main` excecutable requires two arguments, namely the input json and the output file. 
The /filesystem is relative to the directory to which the `main` is excectued from so for example: `out/build/x64-clang-linux-release/main /resources/scenes/textures.json /resources/renders/textures.ppm` should work correctly. 

This version has been tested on linux and works on wsl2 but the version has only been tested with CMake rather than a MakeFile. As such, I hope you have CMake but if not a MakeFile is also provided that might work :)

A Makefile is available so run `make` and the main executable should build within the directory.

<img src="https://github.com/Sean-Leishman/raucous_wart/blob/master/materials/Images/Path1000.png?raw=true">
<img src="https://github.com/Sean-Leishman/raucous_wart/blob/master/materials/Images/RoughMaterial.png?raw=true">
