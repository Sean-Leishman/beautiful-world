#include "shape.hpp"
#include "intersection.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "vector.hpp"

#include <cmath>
#include <utility>

#define EPSILON 0.00000001f

Shape::Shape() {};
Shape::Shape(std::unique_ptr<Material> material)
    : material(std::move(material)) {};

Sphere::Sphere(Vec3 center, float radius, std::unique_ptr<Material> material)
    : center(center), radius(radius), Shape(std::move(material))
{
  bbox = {center - radius, center + radius};
};

std::ostream& operator<<(std::ostream& os, const Shape& shape)
{
  os << "Area: " << shape.name();
  return os;
}

bool Sphere::intersect(const Ray& ray, float tmin, float tmax,
                       Intersection* intersection) const
{
  Vec3 oc = ray.origin - center;
  float a = ray.direction.dot(ray.direction);
  float b = oc.dot(ray.direction);
  float c = oc.dot(oc) - radius * radius;
  float discriminant = b * b - a * c;

  if (discriminant < 0)
    return false;

  float sqrt_disc = std::sqrt(discriminant);
  float temp = (-b - sqrt_disc) / a;
  if (temp < tmax && temp > tmin)
  {
    intersection->distance = temp;
    intersection->position = ray.point_at_parameter(temp);
    intersection->normal =
        (intersection->position - center) / radius; // Already unit length
    intersection->object = get_shared_ptr();
    return true;
  }
  temp = (-b + sqrt_disc) / a;
  if (temp < tmax && temp > tmin)
  {
    intersection->distance = temp;
    intersection->position = ray.point_at_parameter(temp);
    intersection->normal =
        (intersection->position - center) / radius; // Already unit length
    intersection->object = get_shared_ptr();
    return true;
  }
  return false;
}

Vec2 Sphere::interpolate_uv(const Intersection* hit_info) const
{
  Vec3 point = hit_info->position;
  point = point - center;
  point = point * -1;

  float theta = atan2(point.z, point.x);
  float phi = asin(point.y / radius);

  float u = (theta) / (2 * M_PI) + 0.5f;
  float v = phi / (M_PI) + 0.5;

  return {u, v};
}

void Sphere::transform(Mat4& view_matrix)
{
  center = center + view_matrix.translate();
}

Cylinder::Cylinder(Vec3 center, Vec3 axis, float radius, float height,
                   std::unique_ptr<Material> material)
    : Shape(std::move(material)), center(center), axis(axis), radius(radius),
      height(height)
{
  Quaternion rotation = rotation_from_to(Vec3(0.0f, 1.0f, 0.0f), axis);
  Vec3 min = Vec3{static_cast<float>(center.x - radius),
                  static_cast<float>(center.y - height),
                  static_cast<float>(center.z - radius)};
  Vec3 max = Vec3{static_cast<float>(center.x + radius),
                  static_cast<float>(center.y + height),
                  static_cast<float>(center.z + radius)};
  min = rotate_vector(min, rotation);
  max = rotate_vector(max, rotation);
  bbox = {min, max};
};
// Add for rotated cylinders

bool Cylinder::intersect_caps(const Vec3& origin, const Vec3& direction,
                              float t, Intersection* intersection) const
{
  if (direction.y == 0)
  {
    return false;
  }
  float half_height = height;
  float tt = (-height - origin.y) / direction.y;
  if (tt > 0)
  {
    Vec3 p = origin + direction * tt;
    if (p.x * p.x + p.z * p.z <= radius * radius)
    {
      intersection->position = p;
      intersection->distance = tt;
      intersection->normal =
          (p.y > center.y) ? Vec3(0.0f, 1.0f, 0.0f) : Vec3(0.0f, -1.0f, 0.0f);
      return true;
    }
  }

  tt = (height - origin.y) / direction.y;
  if (tt >= 0)
  {
    Vec3 p = origin + direction * tt;
    if (p.x * p.x + p.z * p.z <= radius * radius)
    {
      intersection->position = p;
      intersection->distance = tt;
      intersection->normal =
          (p.y > center.y) ? Vec3(0.0f, 1.0f, 0.0f) : Vec3(0.0f, -1.0f, 0.0f);
      return true;
    }
  }
  return false;
}
bool Cylinder::intersect(const Ray& ray, float tmin, float tmax,
                         Intersection* intersection) const
{
  Quaternion rotation = rotation_from_to(axis, Vec3(0.0f, 1.0f, 0.0f));
  Vec3 oc = rotate_vector(ray.origin - center, rotation);
  Vec3 transform_ray_dir = rotate_vector(ray.direction, rotation);

  Vec3 dir = Vec3::normalize(transform_ray_dir);
  float a = dir.x * dir.x + dir.z * dir.z;
  float b = 2.0f * (oc.x * dir.x + oc.z * dir.z);
  float c = oc.x * oc.x + oc.z * oc.z - radius * radius;

  // Interaction with infinite cylinder
  float discriminant = b * b - 4 * a * c;
  if (discriminant < 0)
  {
    return false; // No intersection
  }

  float sqrt_discriminant = sqrt(discriminant);
  float t1 = (-b - sqrt_discriminant) / (2 * a);
  float t2 = (-b + sqrt_discriminant) / (2 * a);

  // Check if the intersections are within the bounds of the cylinder's height
  float t = t1;
  if (t < tmin || t > tmax)
  {
    t = t2;
    if (t < tmin || t > tmax)
    {
      bool intersect = intersect_caps(oc, dir, t, intersection);
      if (!intersect)
        return false;
      intersection->position =
          rotate_vector(intersection->position, rotation) + center;
      intersection->normal = rotate_vector(intersection->normal, rotation);
      intersection->object = get_shared_ptr();
      intersection->distance = t;
      return true; // Both intersection points are out of bounds
    }
  }

  // Get the intersection point
  Vec3 point = ray.point_at_parameter(oc, transform_ray_dir, t);

  float half_height = height;
  if (point.y < -half_height || point.y > half_height)
  {
    bool intersect = intersect_caps(oc, dir, t, intersection);
    if (!intersect)
      return false;
    intersection->position =
        rotate_vector(intersection->position, rotation) + center;
    intersection->normal = rotate_vector(intersection->normal, rotation);
    intersection->object = get_shared_ptr();
    return true; // Both intersection points are out of bounds
  }

  // Calculate the normal at the intersection point
  Vec3 normal = Vec3::normalize(point - center);
  normal.y = 0;

  point = rotate_vector(point, rotation.conjugate()) + center;
  normal = rotate_vector(normal, rotation.conjugate());

  intersection->position = point;
  intersection->normal = normal;
  intersection->object = get_shared_ptr();
  intersection->distance = t;
  return true;
}

Vec2 Cylinder::interpolate_uv(const Intersection* hit_info) const
{
  Quaternion rotation = rotation_from_to(axis, Vec3(0.0f, 1.0f, 0.0f));
  Vec3 point = hit_info->position;
  point = rotate_vector(point, rotation);
  point = point - center;
  point = point * -1;

  float theta = atan2(point.z, point.x);
  float u = theta / (2 * M_PI) + 0.5f;
  float v = 0.5f + point.y / (2 * height);

  return {u, v};
}

void Cylinder::transform(Mat4& view_matrix)
{
  Vec3 new_axis = view_matrix * axis;
  axis = new_axis;

  center = center + view_matrix.translate();
}

Triangle::Triangle()
    : v0(Vec3()), v1(Vec3()), v2(Vec3()), Shape(std::unique_ptr<Material>())
{
  uv0 = get_uv(v0);
  uv1 = get_uv(v1);
  uv2 = get_uv(v2);
}

Triangle::Triangle(Vec3 v0, Vec3 v1, Vec3 v2,
                   std::unique_ptr<Material> material)
    : v0(v0), v1(v1), v2(v2), Shape(std::move(material))
{
  uv0 = get_uv(v0);
  uv1 = get_uv(v1);
  uv2 = get_uv(v2);

  bbox = {Vec3{std::min({v0.x, v1.x, v2.x}), std::min({v0.y, v1.y, v2.y}),
               std::min({v0.z, v1.z, v2.z})},
          Vec3{std::max({v0.x, v1.x, v2.x}), std::max({v0.y, v1.y, v2.y}),
               std::max({v0.z, v1.z, v2.z})}};
};

bool Triangle::intersect(const Ray& ray, float tmin, float tmax,
                         Intersection* intersection) const
{
  Vec3 edge1 = v1 - v0;
  Vec3 edge2 = v2 - v0;

  Vec3 h = ray.direction.cross(edge2);
  float a = edge1.dot(h);

  if (a > -EPSILON && a < EPSILON)
    return false;

  float f = 1.0 / a;
  Vec3 s = ray.origin - v0;
  float u = f * s.dot(h);

  if (u < 0.0 || u > 1.0)
    return false;

  Vec3 q = s.cross(edge1);
  float v = f * ray.direction.dot(q);

  if (v < 0.0 || u + v > 1.0)
    return false;

  float t = f * edge2.dot(q);
  if (t > tmin && t < tmax)
  {
    intersection->position = ray.point_at_parameter(t);
    intersection->distance = t;
    intersection->normal = Vec3::normalize(edge1.cross(edge2));
    if (intersection->normal.dot(ray.direction) > 0)
    {
      intersection->normal = intersection->normal * -1;
    }
    intersection->object = get_shared_ptr();
    return true;
  }

  return false;
}

Vec2 Triangle::get_uv(const Vec3& point) const
{
  float min_x = std::min({v0.x, v1.x, v2.x});
  float max_x = std::max({v0.x, v1.x, v2.x});
  float min_z = std::min({v0.z, v1.z, v2.z});
  float max_z = std::max({v0.z, v1.z, v2.z});

  float u = (point.x - min_x) / (max_x - min_x);
  float v = (point.z - min_z) / (max_z - min_z);

  return {u, v};
}

Vec2 Triangle::interpolate_uv(const Intersection* hit_info) const
{
  Vec3 point = hit_info->position;

  Vec3 e0 = v2 - v0;
  Vec3 e1 = v1 - v0;
  Vec3 e2 = point - v0;

  float d00 = e0.dot(e0);
  float d01 = e0.dot(e1);
  float d11 = e1.dot(e1);
  float d20 = e2.dot(e0);
  float d21 = e2.dot(e1);
  float denom = d00 * d11 - d01 * d01;

  Vec3 barycentric;
  barycentric.z = (d11 * d20 - d01 * d21) / denom;
  barycentric.y = (d00 * d21 - d01 * d20) / denom;
  barycentric.x = 1 - barycentric.y - barycentric.z;

  float u =
      uv0.u * barycentric.x + uv1.u * barycentric.y + uv2.u * barycentric.z;

  float v =
      uv0.v * barycentric.x + uv1.v * barycentric.y + uv2.v * barycentric.z;

  return {u, v};
}

void Triangle::transform(Mat4& view_matrix)
{
  v0 = view_matrix * v0;
  v1 = view_matrix * v1;
  v2 = view_matrix * v2;

  Vec3 translate = view_matrix.translate();
  v0 = v0 + translate;
  v1 = v1 + translate;
  v2 = v2 + translate;
}
