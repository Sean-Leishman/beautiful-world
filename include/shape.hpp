#pragma once

#include "intersection.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "shape.hpp"
#include "vector.hpp"

struct Vec2
{
  float u;
  float v;

  Vec2 operator*(const float& f) const { return Vec2{u * f, v * f}; }
  Vec2 operator+(const Vec2& f) const { return Vec2{u + f.u, v + f.v}; }
  friend std::ostream& operator<<(std::ostream& out, const Vec2& v)
  {
    out << "(" << v.u << "," << v.v << ")";
    return out;
  }
};

class Intersection;

struct BoundingBox
{
  Vec3 min;
  Vec3 max;

  float EPSILON = 1e-8;
  float t_min = std::numeric_limits<float>::min();
  float t_max = std::numeric_limits<float>::max();

  bool intersect(const Ray& ray)
  {
    Vec3 invDir = {1.0f / ray.direction.x, 1.0f / ray.direction.y,
                   1.0f / ray.direction.z};
    Vec3 t0 = (min - ray.origin) * invDir;
    Vec3 t1 = (max - ray.origin) * invDir;

    Vec3 tminv = t0.min(t1);
    Vec3 tmaxv = t0.max(t1);

    t_min = std::min({tminv.x, tminv.y, tminv.z});
    t_max = std::min({tmaxv.x, tmaxv.y, tmaxv.z});

    return t_max >= t_min;
  }
  Vec3 centroid()
  {
    return {
        (min.x + max.x) / 2,
        (min.y + max.y) / 2,
        (min.z + max.z) / 2,
    };
  }

  void expand_for(const BoundingBox& b)
  {
    min.x = std::min(min.x, b.min.x);
    min.y = std::min(min.y, b.min.y);
    min.z = std::min(min.z, b.min.z);
    max.x = std::max(max.x, b.max.x);
    max.y = std::max(max.y, b.max.y);
    max.z = std::max(max.z, b.max.z);
  }
};

class Shape : public std::enable_shared_from_this<Shape>
{
  private:
  public:
  std::unique_ptr<Material> material;
  BoundingBox bbox;

  Shape();
  Shape(std::unique_ptr<Material> material);
  virtual ~Shape() = default;

  virtual std::shared_ptr<const Shape> get_shared_ptr() const = 0;
  virtual bool intersect(const Ray& ray, float tmin, float tmax,
                         Intersection* intersection) const = 0;
  virtual Vec2 interpolate_uv(const Intersection* hit_info) const = 0;
  virtual std::string name() const = 0;
  friend std::ostream& operator<<(std::ostream& os, const Shape& shape);

  virtual void transform(Mat4& view_matrix) = 0;
};

class Sphere : public Shape
{
  private:
  Vec3 center;
  double radius;

  public:
  Sphere(Vec3, float, std::unique_ptr<Material>);

  std::shared_ptr<const Shape> get_shared_ptr() const override
  {
    return shared_from_this();
  }
  Vec2 interpolate_uv(const Intersection* hit_info) const override;
  bool intersect(const Ray&, float, float, Intersection*) const override;
  std::string name() const override { return "Sphere"; };
  void transform(Mat4& view_matrix) override;
};

class Cylinder : public Shape
{
  private:
  Vec3 center;
  Vec3 axis;
  float radius;
  float height;

  public:
  Cylinder(Vec3, Vec3, float, float, std::unique_ptr<Material>);

  std::shared_ptr<const Shape> get_shared_ptr() const override
  {
    return shared_from_this();
  }

  Vec2 interpolate_uv(const Intersection* hit_info) const override;
  bool intersect_caps(const Vec3&, const Vec3&, float, Intersection*) const;
  bool intersect(const Ray& ray, float tmin, float tmax,
                 Intersection* intersection) const override;
  std::string name() const override { return "Cylinder"; };
  void transform(Mat4& view_matrix) override;
};

class Triangle : public Shape
{
  private:
  Vec3 v0;
  Vec3 v1;
  Vec3 v2;

  Vec2 uv0;
  Vec2 uv1;
  Vec2 uv2;

  public:
  Triangle();
  Triangle(Vec3, Vec3, Vec3, std::unique_ptr<Material>);

  std::shared_ptr<const Shape> get_shared_ptr() const override
  {
    return shared_from_this();
  }

  Vec2 get_uv(const Vec3& point) const;
  bool intersect(const Ray& ray, float tmin, float tmax,
                 Intersection* intersection) const override;
  Vec2 interpolate_uv(const Intersection* hit_info) const override;
  std::string name() const override { return "Trigangle"; };
  void transform(Mat4& view_matrix) override;
};
