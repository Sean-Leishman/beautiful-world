# Architecture

## High-level flow

```
main.cpp
  -> Renderer::load_file(input.json)
       -> JSONParser parses scene
       -> builds Camera, Scene, Materials, Shapes, Lights
       -> Scene::build_bvh()
  -> Renderer::render_frame(output.ppm)
       -> picks Raytracer subclass based on rendermode
       -> for each pixel (parallel via OpenMP):
            Raytracer::trace_ray(x, y) -> PPMColor
       -> PPMImage writes the file
```

`main.cpp` is intentionally tiny: parse args, instantiate `Renderer`, call
`load_file` then `render_frame`. All real work is inside the `raytracer_lib`
static library produced by `src/CMakeLists.txt`.

## Components

### Renderer (`include/renderer.hpp`, `src/renderer.cpp`)
Owns the `Scene`, `Camera` (`PRenderHole`), `PPMImage`, and the active
`Raytracer`. Loads materials and textures into an `unordered_map` keyed by
some material name, and assembles shapes/lights from the JSON.

### Raytracer hierarchy (`include/raytracer.hpp`)
- `Raytracer` - abstract base with `trace_ray(float x, float y)`
- `BinaryRaytracer` - returns a flat colour on hit; quick visibility test
- `PhongRaytracer` - recursive Whitted-style with reflection / refraction;
  `max_depth` from `nbounces`
- `Pathtracer` - Monte Carlo integrator with `n_samples` per pixel and
  recursive `trace_ray(Ray&, depth)`; computes direct lighting separately
  via `calculate_direct_lighting`

### Scene (`include/scene.hpp`)
Holds shapes, lights, an ambient light, background colour, and a `BVHTree`.
`intersect` falls back to linear iteration; `intersect_bvh` uses the BVH.
`apply_transform` lets you push a view matrix through every shape.

### BVH (`include/bvh.hpp`, `src/bvh.cpp`)
Bounding-volume hierarchy used by `Scene::intersect_bvh`. Built once in
`Scene::build_bvh()` after all shapes are loaded.

### Shapes (`include/shape.hpp`, `src/shape.cpp`)
Polymorphic `Shape` base. JSON supports at least `sphere` and `triangle`
(used heavily by `scripts/animation.py` for floors and walls). Cylinders may
also be present - check `shape.cpp` to confirm.

### Materials & textures (`include/material.hpp`)
Phong-style parameters: `kd`, `ks`, `specularexponent`, `diffusecolor`,
`specularcolor`, `isreflective`, `reflectivity`, `isrefractive`,
`refractiveindex`. Textures are PPM images loaded via the `textures` map.

### Lights (`include/light.hpp`)
Point lights and area lights; the area light is what the path tracer samples
for direct illumination.

### Camera (`include/camera.hpp`)
Pinhole camera (`PRenderHole`). JSON fields: `width`, `height`, `position`,
`lookAt`, `upVector`, `fov`, `exposure`.

### Image (`include/image.hpp`)
PPM writer. Format is plain ASCII / binary PPM - matches the `*.ppm`
artefacts under `materials/`.

### Math (`include/vector.hpp`, `include/transform.hpp`)
Vec3 / Mat4 helpers used throughout.

### JSON (`include/json.hpp`, `include/input.hpp`)
`include/json.hpp` is vendored `nlohmann::json`. `JSONParser` in `input.hpp`
wraps it for scene loading.

## Build graph

`CMakeLists.txt` (root) configures clang-21, C++20, Release/Debug/RelWithDebInfo
flags, finds OpenMP, then `add_subdirectory(src)` to build `raytracer_lib`
(GLOB of every `src/*.cpp`). The root `add_executable(main ...)` links
`main.cpp` against `raytracer_lib` and `OpenMP::OpenMP_CXX`. LTO is enabled
for Release if `CheckIPOSupported` passes.

Presets (`CMakePresets.json`):
- `default` / `release` - Ninja, Release, clang-21
- `debug` - Debug + AddressSanitizer + UBSan
- `release-lto` - Release with `CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`

## Parallelism

`main.cpp` prints `omp_get_max_threads()` on startup. The per-pixel render
loop in `Renderer::render_frame` is what OpenMP parallelises (look at
`src/renderer.cpp` for the `#pragma omp` directives if you need detail).

## Scene JSON shape

Top-level keys observed in `examples/` and `scripts/animation.py`:

- `rendermode` - `"binary"` | `"phong"` | `"pathtracer"`
- `nbounces` - recursion / path depth (phong & pathtracer)
- `nsamples` - samples per pixel (pathtracer)
- `camera` - object as described above
- `scene.backgroundcolor` - `[r, g, b]`
- `scene.lightsources` - list of lights (`pointlight`, `arealight`)
- `scene.shapes` - list of shapes with embedded `material`
