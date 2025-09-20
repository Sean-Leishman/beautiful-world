#include "vector.hpp"

Vec3::Vec3() : x(0), y(0), z(0) {};

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {};
Vec3::Vec3(double x, double y, double z) : x(x), y(y), z(z) {};

Vec3::Vec3(const std::vector<float>& vec) : x(vec[0]), y(vec[1]), z(vec[2]) {};

Quaternion rotation_from_to(Vec3 from, Vec3 to)
{
  from = Vec3::normalize(from);
  to = Vec3::normalize(to);
  float cosTheta = from.dot(to);
  Vec3 rotationAxis;

  if (cosTheta < -1 + 0.001f)
  {
    // special case when vectors in opposite directions:
    rotationAxis = Vec3(0.0f, 0.0f, 1.0f).cross(from);
    if (rotationAxis.length() < 0.01) // they were parallel, try again!
      rotationAxis = Vec3(1.0f, 0.0f, 0.0f).cross(from);

    rotationAxis = Vec3::normalize(rotationAxis);
    return Quaternion(rotationAxis.x, rotationAxis.y, rotationAxis.z,
                      0); // 180 degree rotation
  }

  rotationAxis = from.cross(to);
  float angle = std::acos(cosTheta);

  float sin_angle = std::sin(angle / 2);

  Quaternion quat(std::cos(angle / 2), rotationAxis.x * sin_angle,
                  rotationAxis.y * sin_angle, rotationAxis.z * sin_angle);
  quat.normalize();
  return quat;
}

Vec3 rotate_vector(const Vec3& v, const Quaternion& q)
{
  Quaternion qv(0, v.x, v.y, v.z);
  Quaternion qv_rotated = q * qv * q.conjugate();
  return {qv_rotated.x, qv_rotated.y, qv_rotated.z};
}