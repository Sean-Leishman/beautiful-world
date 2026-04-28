# Notes

Loose observations and unknowns. Personal scratch space.

## Naming

- Repo / Obsidian name: `beautiful-world`.
- CMake project name: `raucous_wart` (and the README still links to a
  `Sean-Leishman/raucous_wart` GitHub repo).
- Camera type is `PRenderHole` (pinhole, but the spelling is a deliberate
  pun).

## Render modes

Set via `"rendermode"` in the scene JSON:

- `binary` - hit / no-hit colouring; fastest, useful as a sanity check.
- `phong` - Whitted-style, supports reflection and refraction; `nbounces`
  controls recursion depth.
- `pathtracer` - Monte Carlo with `nsamples` samples per pixel and `nbounces`
  bounces. Direct lighting is computed separately
  (`Pathtracer::calculate_direct_lighting`).

## Things that look stale or inconsistent

- `README.md` references `./cmake.bash`, `cmake_debug.bash`, and the old
  output path `out/build/x64-clang-linux-release/main`. The current canonical
  flow is `make` (or `cmake --preset default`) producing `build/default/main`.
  The shell scripts now live in `archive/`.
- `scripts/animation.py` hard-codes `./cmake-build-release/main`.
- The `make example-*` targets pass `/examples/...` (leading slash). Confirm
  whether they actually work from the repo root before relying on them.
- README mentions clang++-14 as preferred, but `CMakeLists.txt` and presets
  pin clang-21. The CMake config is the source of truth.

## Unknowns / TODO when revisiting

- Exact set of supported shape types in JSON. `sphere` and `triangle` are
  confirmed from `examples/` and `scripts/animation.py`. Cylinders, planes,
  meshes - check `src/shape.cpp` and `src/input.cpp`.
- Texture loading: `Renderer::textures` is keyed by string but the JSON
  schema for textures is not documented here. `materials/earth.ppm` and the
  `cornell.json` / `pathtracer.json` files in `materials/` are good examples.
- BVH construction strategy (`src/bvh.cpp`) - SAH? Median split? Not checked.
- Tone mapping / exposure: camera has an `exposure` field but how it is
  applied is in `src/renderer.cpp` / `src/image.cpp`.
- spdlog: `cmake/Findspdlog.cmake` exists but is not obviously used in the
  top-level CMakeLists; might be a leftover or used conditionally inside
  `src/`.

## Useful starting points when reading the code

1. `main.cpp` -> tiny, just sets up Renderer.
2. `src/renderer.cpp` -> read `load_file` and `render_frame` to understand
   the JSON-to-pixels pipeline.
3. `src/raytracer.cpp` -> the three `trace_ray` implementations.
4. `src/scene.cpp` + `src/bvh.cpp` -> intersection.
5. `examples/pathtracer_cornell.json` -> realistic input shape.

## Performance knobs

- `release-lto` preset for max throughput.
- OpenMP thread count via `OMP_NUM_THREADS`.
- `-march=native` is on in the Release preset's `CMAKE_CXX_FLAGS_RELEASE`,
  along with `-ffast-math` and SLP/tree vectorisation flags. Be aware that
  `-ffast-math` can change numerical behaviour - if a render looks subtly
  off, try `RelWithDebInfo` (just `-O2 -g -march=native`) and compare.

## Reference images

`materials/Images/` (referenced from the README) holds the `Path1000.png` and
`RoughMaterial.png` showcase images. The README links them via the upstream
GitHub repo URL.
