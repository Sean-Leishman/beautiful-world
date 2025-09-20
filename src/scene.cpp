#include "scene.hpp"
#include "bvh.hpp"
#include "intersection.hpp"
#include "ray.hpp"
#include "shape.hpp"

#include <any>
#include <limits>
#include <memory>
#include <vector>

Scene::Scene()
{
  bg_color = {0, 0, 0};
  shapes = std::vector<std::shared_ptr<Shape>>();
}

void Scene::apply_transform(Mat4& view_matrix)
{
  for (auto& object : shapes)
  {
    object->transform(view_matrix);
  }

  for (auto& object : lights)
  {
    object->transform(view_matrix);
  }
}

void Scene::build_bvh() { bvh_tree.buildBVH(shapes); }

void Scene::add_shape(std::shared_ptr<Shape> shape) { shapes.push_back(shape); }

bool Scene::intersect_bvh(const Ray& ray, Intersection& intersection)
{
  Ray ray_copy = ray; // BVH traversal modifies ray.hit_distance
  return bvh_tree.intersectBVH(ray_copy, intersection);
}

bool Scene::intersect(const Ray& ray, Intersection& return_intersection)
{
  bool hit = false;
  float hit_distance = std::numeric_limits<float>::max();

  for (const auto& object : shapes)
  {
    Intersection intersection;
    if (object->intersect(ray, 0.001f, hit_distance, &intersection))
    {
      hit = true;
      if (intersection.distance < hit_distance)
      {
        hit_distance = intersection.distance;
        return_intersection = intersection;
      }
    }
  }

  return hit;
}
bool Scene::object_in_shadow(const Ray& shadow_ray,
                             std::shared_ptr<const Shape> shadow_object,
                             Intersection* shadow_hit)
{
  float hit_distance = std::numeric_limits<float>::max();

  for (const auto& object : shapes)
  {
    if (object == shadow_object)
    {
      continue;
    }
    Intersection intersection;
    if (object->intersect(shadow_ray, 0.0001f, hit_distance, &intersection))
    {
      shadow_hit->object = object;
      return true;
    }
  }
  return false;
}
