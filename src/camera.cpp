#include "camera.hpp"
#include "random.h"
#include "ray.hpp"
#include "raytracer.hpp"
#include "vector.hpp"

#include <numbers>

PRenderHole::PRenderHole()
{
  width = 0;
  height = 0;
  position = Vec3();
  lookAt = Vec3();
  upVector = Vec3();
  fov = 0;
  exposure = 0;

  right = Vec3();
  up = Vec3();
  forward = Vec3();

  view_matrix = Mat4();

  defocus_angle = 0;
  focus_dist = 10;
  defocus_u = Vec3();
  defocus_v = Vec3();

  defocus = false;
};

PRenderHole::PRenderHole(int width, int height, Vec3 position, Vec3 lookAt,
                         Vec3 upVector, float fov, float exposure)
    : width(width), height(height), position(position), lookAt(lookAt),
      upVector(upVector), fov(fov), exposure(exposure), defocus_angle(0.3),
      focus_dist(20), defocus_u(Vec3()), defocus_v(Vec3()), defocus(false)
{
  forward = Vec3::normalize(position - lookAt);
  right = Vec3::normalize(upVector.cross(forward));
  up = forward.cross(right);

  view_matrix =
      Mat4{right.x,   right.y,   right.z,   right.dot(position) * -1,
           up.x,      up.y,      up.z,      up.dot(position) * -1,
           forward.x, forward.y, forward.z, forward.dot(position) * -1,
           0.0,       0.0,       0.0,       1.0};
  transform_matrix = view_matrix.inverse();

  aspect = width / (float)height;

  float theta = fov * M_PI / 180.0f;
  half_height = 2 * std::tan(theta / 2.0f) * focus_dist;
  half_width = aspect * half_height;

  viewport_u = right * half_width;
  viewport_v = up * -1 * half_height;

  pixel_delta_u = viewport_u / width;
  pixel_delta_v = viewport_v / height;

  auto viewport_upper_left =
      position - (forward * focus_dist) - (viewport_u / 2) - (viewport_v / 2);
  pixel100_loc = viewport_upper_left + (pixel_delta_u + pixel_delta_v) * 0.5;

  auto defocus_radius = focus_dist * tan((defocus_angle / 2) * M_PI / 180.0f);
  defocus_u = right * defocus_radius;
  defocus_v = up * defocus_radius;

  if (!defocus)
  {
    defocus_angle = -0.1;
  }
};

Ray PRenderHole::compute_ray(float s, float t)
{
  /*
  s = (s / (float)width) * 2 - 1;
  t = (t / (float)height) * 2 - 1;

  s *= aspect;
   */

  auto pixel_center = pixel100_loc + (pixel_delta_u * s) + (pixel_delta_v * t);
  auto pixel_sample = pixel_center + pixel_sample_square();
  if (!defocus)
  {
    pixel_sample = pixel_center;
  }

  auto ray_origin = (defocus_angle <= 0) ? position : defocus_disk_sample();
  auto ray_direction = pixel_sample - ray_origin;
  // auto ray_direction = pixel_sample;
  //  Vec3 ray_direction = right * half_width * s + up * half_width * t -
  //  forward;

  ray_direction = Vec3::normalize(ray_direction);

  return Ray{position, ray_direction};
  // return Ray{position-position, ray_direction  + ray_origin};
};
