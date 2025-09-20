#include "light.hpp"
#include "vector.hpp"

Light::Light(Vec3 position, Vec3 intensity, PPMColor color)
    : position(position), intensity(intensity), color(color) {};
Light::Light(Vec3 position, Vec3 intensity)
    : position(position), intensity(intensity), color(PPMColor(254, 255, 255))
{
}

PointLight::PointLight(Vec3 position, Vec3 intensity)
    : Light(position, intensity) {};
float PointLight::compute_intensity(float NdotL)
{
  return std::max(std::min(NdotL, 1.0f), 0.0f);
}
Vec3 PointLight::get_position() { return position; }

void PointLight::transform(Mat4& view_matrix)
{
  Vec3 translate = view_matrix.translate();
  position = position + translate;
}

AmbientLight::AmbientLight(Vec3 intensity, PPMColor color)
    : Light(Vec3(), intensity, color)
{
}
float AmbientLight::compute_intensity(float) { return 0; };
Vec3 AmbientLight::get_position() { return position; }

void AmbientLight::transform(Mat4&) { return; }

AreaLight::AreaLight(Vec3 position, Vec3 intensity, Vec3 normal, Vec3 size)
    : Light(position, intensity), normal(normal), size(size)
{
  Vec3 axis = (fabs(normal.y) < 0.9999) ? Vec3(0.0f, 1.0f, 0.0f)
                                        : Vec3(1.0f, 0.0f, 0.0f);

  u = Vec3::normalize(normal.cross(axis));
  v = normal.cross(u);
}

float AreaLight::compute_intensity(float) { return 0; }
Vec3 AreaLight::get_position()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(-0.5, 0.5);
  float random_u = dis(gen) * size.x;
  float random_v = dis(gen) * size.y;
  return position + (u * random_u) + (v * random_v);
}

void AreaLight::transform(Mat4& view_matrix)
{
  Vec3 translate = view_matrix.translate();
  position = position + translate;
}
