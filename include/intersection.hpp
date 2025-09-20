#pragma once

#include "input.hpp"
#include "vector.hpp"

#include "memory"

enum IntersectionType
{
  NORMAL,
  CAPS,
};

class Shape;

class Intersection
{
  public:
  Vec3 normal;
  Vec3 position;
  float distance{};
  IntersectionType type;
  bool first_hit;

  std::shared_ptr<const Shape> object;

  Intersection() : type(NORMAL), first_hit(true){};
};
