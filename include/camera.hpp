#pragma once

#include "random.h"
#include "ray.hpp"
#include "vector.hpp"

class Camera
{
  public:
  virtual Vec3 get_position() = 0;
};

class PRenderHole : public Camera
{
  private:
  int width;
  int height;
  Vec3 position;
  Vec3 lookAt;
  Vec3 upVector;
  float fov;

  float defocus_angle;
  float focus_dist;

  Vec3 defocus_u;
  Vec3 defocus_v;

  Vec3 right;
  Vec3 up;
  Vec3 forward;

  float aspect;

  float half_height;
  float half_width;

  Vec3 viewport_u;
  Vec3 viewport_v;

  Vec3 pixel_delta_u;
  Vec3 pixel_delta_v;

  Vec3 pixel100_loc;

  public:
  float exposure;
  Mat4 view_matrix;
  Mat4 transform_matrix;
  bool defocus;

  PRenderHole();
  PRenderHole(int, int, Vec3, Vec3, Vec3, float, float);

  Ray compute_ray(float, float);

  Vec3 get_position() override { return position; };

  Vec3 pixel_sample_square()
  {
    // Returns a random point in the square surrounding a pixel at the origin.
    auto px = -0.5 + random_double();
    auto py = -0.5 + random_double();
    return (pixel_delta_u * px) + (pixel_delta_v * py);
  }

  Vec3 pixel_sample_disk(double radius) const
  {
    // Generate a sample from the disk of given radius around a pixel at the
    // origin.
    auto p = Vec3::random_in_unit_disk() * radius;
    return (pixel_delta_u * p.x) + (pixel_delta_v * p.y);
  }

  Vec3 defocus_disk_sample() const
  {
    // Returns a random point in the camera defocus disk.
    auto p = Vec3::random_in_unit_disk();
    return (defocus_u * p.x) + (defocus_v * p.y);
  }
};