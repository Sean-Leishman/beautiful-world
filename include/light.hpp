#pragma once

#include "image.hpp"
#include "vector.hpp"

enum LightType
{
  POINT,
};

class Light
{
  public:
  Vec3 position;
  Vec3 intensity;
  PPMColor color;

  Light(Vec3, Vec3, PPMColor);
  Light(Vec3, Vec3);
  virtual ~Light() {};

  virtual float compute_intensity(float) = 0;
  virtual Vec3 get_position() = 0;
  virtual void transform(Mat4&) = 0;
};

class PointLight : public Light
{
  public:
  ~PointLight() = default;
  PointLight();
  PointLight(Vec3 position, Vec3 intensity);
  float compute_intensity(float) override;
  Vec3 get_position() override;
  void transform(Mat4&) override;
};

class AmbientLight : public Light
{
  public:
  ~AmbientLight() = default;
  AmbientLight();
  AmbientLight(Vec3 intensity, PPMColor color);
  float compute_intensity(float) override;
  Vec3 get_position() override;
  void transform(Mat4&) override;
};

class AreaLight : public Light
{
  public:
  Vec3 normal;
  Vec3 size;
  Vec3 u;
  Vec3 v;

  AreaLight();
  AreaLight(Vec3 position, Vec3 intensity, Vec3 normal, Vec3 size);

  float compute_intensity(float) override;
  Vec3 get_position() override;
  void transform(Mat4&) override;
};
