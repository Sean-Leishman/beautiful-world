# beautiful-world

## Purpose

A C++20 ray tracer / path tracer that reads a JSON scene description and writes
a PPM image. Internally the project is named `raucous_wart`. It supports three
render modes: `binary` (visibility), `phong` (Whitted-style with reflection and
refraction), and `pathtracer` (Monte Carlo with N samples per pixel and an area
light). Spheres, triangles, and a BVH for intersection acceleration are
implemented; OpenMP parallelises the per-pixel work.

## Tech stack

- C++20, built with `clang-21` / `clang++-21` (hard-coded in `CMakeLists.txt`)
- CMake 3.16+ with presets, Ninja generator, optional LTO
- OpenMP (`libomp-dev` / `-fopenmp`) for parallelism
- `nlohmann::json` (vendored at `include/json.hpp`)
- Optional spdlog discovery via `cmake/Findspdlog.cmake`
- Python 3 helper scripts (`scripts/animation.py`, `scripts/gpt_json.py`) which
  shell out to `ffmpeg` for the animation pipeline
- clang-format-21 / clang-tidy-21 for lint (configs in repo root)

## Key files / entry points

- `main.cpp` - CLI entry: takes `<input.json>` and `<output.ppm>`
- `include/renderer.hpp`, `src/renderer.cpp` - top-level Renderer that loads
  scene JSON and drives rendering
- `include/raytracer.hpp` - `Raytracer` base + `BinaryRaytracer`,
  `PhongRaytracer`, `Pathtracer` subclasses
- `include/scene.hpp` / `src/scene.cpp` - scene graph + BVH wiring
- `include/bvh.hpp`, `include/shape.hpp`, `include/material.hpp`,
  `include/light.hpp`, `include/camera.hpp`, `include/image.hpp`
- `src/CMakeLists.txt` - builds everything in `src/*.cpp` into `raytracer_lib`
- `examples/*.json` - canonical scene files (`simple_binary`, `phong_spheres`,
  `pathtracer_cornell`)
- `materials/` - extra hand-written scenes, textures (`*.ppm`), and reference
  renders (large; not all gitignored)
- `archive/cmake.bash`, `archive/cmake_release.bash` - older build scripts that
  the README refers to

## How to run / dev

```bash
# Configure + build (default = release)
make            # or: cmake --preset default && cmake --build --preset default

# Build variants
make debug      # AddressSanitizer + UBSan
make release
make release-lto

# Render an example scene
make example-binary
make example-phong
make example-pathtracer
make examples

# Manual invocation
./build/default/main examples/simple_binary.json output/out.ppm

# Lint
make format            # apply clang-format
make format-check
make tidy              # clang-tidy static analysis
make lint              # format-check + tidy
```

## Conventions / things to know

- Build output lives under `build/<preset>/` (Ninja). The README still mentions
  `out/build/x64-clang-linux-release/` and `cmake-build-release/`, which are
  stale paths.
- Compiler is pinned to `clang-21` in both `CMakeLists.txt` and presets;
  swapping to GCC would need edits.
- `make example-*` targets pass paths with leading `/` (`/examples/...`,
  `/output/...`) which look wrong but are how the makefile is written - run
  from the repo root and they resolve as relative because of how the shell
  joins them; flagged here because it is suspicious.
- `.clang-format` and `.clang-tidy` exist; respect them before editing source.
- Source style: braces on own line (Allman-ish), `private:` indented inside
  classes (see `renderer.hpp`).
- `scripts/animation.py` hard-codes `./cmake-build-release/main` - it predates
  the CMake presets layout and will need its path updated to use it.
