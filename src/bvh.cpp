#include "bvh.hpp"

#include <memory>
#include <vector>

BVHNode::BVHNode(std::vector<std::shared_ptr<Shape>> shapes)
    : objects(std::move(shapes)) {};
void BVHTree::buildBVH(std::vector<std::shared_ptr<Shape>> objects)
{
  root = _buildBVH(objects);
}

std::unique_ptr<BVHNode>
BVHTree::_buildBVH(std::vector<std::shared_ptr<Shape>> objects)
{
  if (objects.size() <= threshold)
  {
    auto node = std::make_unique<BVHNode>(std::move(objects));
    /*
    for (const auto& obj: node->objects){
      node->bbox.expand_for(obj->bbox);
    }
    */
    return node;
  }

  auto [leftObjects, rightObjects] = split_objects(objects);

  if (leftObjects.size() == 0 || rightObjects.size() == 0)
  {
    if (leftObjects.size() > 0)
    {
      auto node = std::make_unique<BVHNode>(std::move(leftObjects));
      return node;
    }
    else
    {
      auto node = std::make_unique<BVHNode>(std::move(rightObjects));
      return node;
    }
  }

  std::unique_ptr<BVHNode> leftChild = _buildBVH(leftObjects);
  std::unique_ptr<BVHNode> rightChild = _buildBVH(rightObjects);

  auto node = std::make_unique<BVHNode>();
  node->left = std::move(leftChild);
  node->right = std::move(rightChild);
  update_bounding_box(node.get());

  return node;
}

void BVHTree::update_bounding_box(BVHNode* node)
{
  if (!node)
    return;

  BoundingBox new_box;
  new_box.max = {std::numeric_limits<float>::min(),
                 std::numeric_limits<float>::min(),
                 std::numeric_limits<float>::min()};
  new_box.min = {std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max()};

  if (node->left)
  {
    new_box.expand_for(node->left->bbox);
  }
  if (node->right)
  {
    new_box.expand_for(node->right->bbox);
  }

  if (node->is_leaf())
  {
    for (const auto& object : node->objects)
    {
      new_box.expand_for(object->bbox);
    }
  }

  node->bbox = new_box;
}

std::pair<BVHTree::Shapes, BVHTree::Shapes>
BVHTree::split_objects(BVHTree::Shapes& objects)
{
  char* axis = "xyz";

  std::vector<float> centroids;

  for (size_t i = 0; i < 3; ++i)
  {
    centroids.reserve(objects.size());
    for (const auto& obj : objects)
    {
      centroids.push_back(obj->bbox.centroid().get(axis[i]));
    }

    std::nth_element(centroids.begin(),
                     centroids.begin() + centroids.size() / 2, centroids.end());
    float median = centroids[centroids.size() / 2];

    // Split objects into two groups based on the median
    std::vector<std::shared_ptr<Shape>> leftObjects, rightObjects;
    for (size_t i = 0; i < objects.size(); ++i)
    {

      if (objects[i]->bbox.centroid().get(axis[i]) < median)
      {
        leftObjects.push_back(std::move(objects[i]));
      }
      else
      {
        rightObjects.push_back(std::move(objects[i]));
      }
    }
    return {std::move(leftObjects), std::move(rightObjects)};
  }
}

bool BVHTree::intersectBVH(Ray& ray, Intersection& hit_info)
{
  return intersectBVH(root.get(), ray, hit_info);
}

bool BVHTree::intersectBVH(BVHNode* node, Ray& ray, Intersection& hit_info)
{
  if (!node || !node->bbox.intersect(ray))
    return false;

  if (node->is_leaf())
  {
    float hit_distance = ray.hit_distance;

    Intersection rt;
    bool hit = false;
    for (auto const& obj : node->objects)
    {
      if (obj->intersect(ray, 0.0001f, hit_distance, &rt))
      {
        if (rt.distance < hit_distance)
        {
          hit = true;
          hit_distance = rt.distance;
          hit_info = rt;

          ray.hit_distance = hit_distance;
        }
      }
    }
    return hit;
  }

  bool hit_left = intersectBVH(node->left.get(), ray, hit_info);
  bool hit_right = intersectBVH(node->right.get(), ray, hit_info);
  return hit_left || hit_right;
}
