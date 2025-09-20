#pragma once

#include "vector.hpp"

class Transform{
  public:
      Vec3 position;
      Quaternion rotation;
      Vec3 scale;

      Transform() = default;

      Vec3 inverse_transform_point(const Vec3& point) const {
        return point;
      }

      Vec3 inverse_transform_direction(const Vec3& direction) const {
        return direction;
      }


};