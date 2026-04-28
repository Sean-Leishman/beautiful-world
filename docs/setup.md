# Setup

## System requirements

- Linux (tested on WSL2 per the README) or anywhere clang-21 + OpenMP work
- `clang-21` and `clang++-21` on PATH - the build is pinned to this version in
  `CMakeLists.txt` and `CMakePresets.json`. Downgrading to clang-14, clang-18,
  etc. requires editing both files.
- `libomp-dev` (Debian/Ubuntu) or equivalent. If CMake fails to find OpenMP:
  ```bash
  sudo apt-get install libomp-dev
  ```
- CMake 3.16+ (3.20+ for `--list-presets=all`)
- Ninja (`sudo apt-get install ninja-build`) - selected by every preset
- `clang-format-21`, `clang-tidy-21` for `make lint` (override via the
  `CLANG_FORMAT` / `CLANG_TIDY` makefile variables if you have a different
  version)
- Python 3 + ffmpeg if you want to use `scripts/animation.py`

## First build

```bash
cd /home/seanleishman/personal/beautiful-world
make            # configure + build with the `default` preset (Release)
```

Equivalent without the makefile wrapper:

```bash
cmake --preset default
cmake --build --preset default
```

The resulting binary is at `build/default/main`.

## Build variants

| Target | Preset | Notes |
|--------|--------|-------|
| `make` / `make build` | `default` | Release, clang-21, Ninja |
| `make debug` | `debug` | `-g -O0` + AddressSanitizer + UBSan |
| `make release` | `release` | Release |
| `make release-lto` | `release-lto` | Release + IPO/LTO |

`make clean` removes `build/`.

## Running a scene

The binary takes two positional args: the input JSON and the output PPM.

```bash
./build/default/main examples/simple_binary.json output/binary.ppm
./build/default/main examples/phong_spheres.json output/phong.ppm
./build/default/main examples/pathtracer_cornell.json output/cornell.ppm
```

Convenience targets, which create `output/` and run the three examples:

```bash
make example-binary
make example-phong
make example-pathtracer
make examples
```

Note: the example targets pass paths starting with `/` (e.g.
`/examples/simple_binary.json`). This is in the makefile as written; if a
target fails to find the input, run the binary manually from the repo root
with relative paths instead.

## Viewing PPM output

PPMs are not viewable by most image apps. Convert with ImageMagick:

```bash
convert output/binary.ppm output/binary.png
```

Or `feh output/binary.ppm` works directly.

## Lint and format

```bash
make format          # apply clang-format-21 to src/, include/, main.cpp
make format-check    # error if anything would change
make tidy            # clang-tidy-21 against the default build's compile_commands.json
make tidy-fix        # apply clang-tidy fixits
make lint            # format-check + tidy
```

`make tidy` depends on `make configure`, which produces
`build/default/compile_commands.json`.

## Animation pipeline (optional)

`scripts/animation.py` generates 100 frames of JSON, renders each one, then
calls `ffmpeg` to stitch a `materials/Animation/video.mp4`. Out of the box it
expects the binary at `./cmake-build-release/main`, which no longer matches
the preset layout - either symlink that path to `build/default/main` or edit
the script before running.

## Output / artefact directories

- `build/<preset>/` - CMake build trees (gitignored)
- `output/` - default place for renders (gitignored)
- `materials/` - hand-curated scenes, textures (`*.ppm`), and reference images.
  Most `*.ppm` files are gitignored, but committed example outputs may exist.
- `archive/` - older bash build scripts and a sample render
