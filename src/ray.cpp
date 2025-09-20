#include "ray.hpp"

Ray::Ray() : hit_distance(std::numeric_limits<float>::max()) {};
Ray::Ray(Vec3 origin, Vec3 dir)
    : origin(origin), direction(dir),
      hit_distance(std::numeric_limits<float>::max()) {};

Vec3 Ray::point_at_parameter(float dist) const
{
  return origin + direction * dist;
}

Vec3 Ray::point_at_parameter(Vec3 origin, Vec3 dir, float dist) const
{
  return origin + dir * dist;
}

LightRay::LightRay(Vec3 origin, Vec3 dir) : Ray(origin, dir) {};

ShadowRay::ShadowRay(Vec3 origin, Vec3 dir) : Ray(origin, dir) {};