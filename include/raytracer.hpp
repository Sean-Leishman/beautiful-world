#pragma once

#include "camera.hpp"
#include "image.hpp"
#include "intersection.hpp"
#include "ray.hpp"
#include "scene.hpp"

class Raytracer
{
  public:
  virtual ~Raytracer() = default;

  Scene* scene;
  PRenderHole* camera;

  Raytracer(Scene* scene, PRenderHole* camera)
      : scene(scene), camera(camera) {};

  virtual PPMColor trace_ray(float x, float y) = 0;
};

class BinaryRaytracer : public Raytracer
{
  public:
  BinaryRaytracer(Scene* scene, PRenderHole* camera)
      : Raytracer(scene, camera) {};
  PPMColor trace_ray(float x, float y) override;
};

class PhongRaytracer : public Raytracer
{
  public:
  int max_depth;

  PhongRaytracer(Scene* scene, PRenderHole* camera, int nbounces)
      : Raytracer(scene, camera), max_depth(nbounces)
  {
  }
  PPMColor trace_ray(float x, float y) override;
  Vec3 trace_ray(Ray& ray, int depth, Intersection& hit_info);
  Vec3 calculate_direct(Intersection& hit_info);
  Ray calculate_reflection_ray(Ray& ray, Intersection& hit_info);
  Ray calculate_refraction_ray(Ray& ray, Intersection& hit_info, double ir);
};

class Pathtracer : public Raytracer
{
  public:
  int max_depth;
  int n_samples;

  Pathtracer(Scene* scene, PRenderHole* camera, int nbounces, int n_samples)
      : Raytracer(scene, camera), max_depth(nbounces), n_samples(n_samples) {};

  PPMColor trace_ray(float x, float y) override;
  Vec3 trace_ray(Ray& ray, int depth);
  Vec3 calculate_direct_lighting(Ray& ray, Intersection& hit_info);
};
