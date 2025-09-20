#pragma once

#include "intersection.hpp"
#include "ray.hpp"
#include "shape.hpp"
#include "vector.hpp"

#include <memory>
#include <utility>

class BVHNode
{
  public:
  BoundingBox bbox;
  std::unique_ptr<BVHNode> left;
  std::unique_ptr<BVHNode> right;
  std::vector<std::shared_ptr<Shape>> objects;

  BVHNode() = default;
  BVHNode(std::vector<std::shared_ptr<Shape>> shapes);

  bool is_leaf() const { return objects.size() > 0; }
};

class BVHTree
{
  using Shapes = std::vector<std::shared_ptr<Shape>>;

  private:
  public:
  BVHTree() : threshold(1), root(std::make_unique<BVHNode>()) {};
  float threshold;
  std::unique_ptr<BVHNode> root;

  void buildBVH(std::vector<std::shared_ptr<Shape>> objects);
  void update_bounding_box(BVHNode* node);

  bool intersectBVH(BVHNode* node, Ray& ray, Intersection& hit_info);
  bool intersect_bvh(Ray ray, Intersection& intersection);
  std::pair<Shapes, Shapes> split_objects(Shapes& objects);
  bool intersectBVH(Ray& ray, Intersection& hit_info);
  std::unique_ptr<BVHNode>
  _buildBVH(std::vector<std::shared_ptr<Shape>> objects);
};
