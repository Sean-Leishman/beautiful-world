#pragma once

#include "random.h"
#include <cmath>
#include <iostream>
#include <vector>

class Vec3
{
  public:
  double x;
  double y;
  double z;
  Vec3();
  Vec3(std::vector<float>);
  Vec3(float, float, float);
  Vec3(double x, double y, double z);

  Vec3 operator-(const Vec3& v) const
  {
    return Vec3(x - v.x, y - v.y, z - v.z);
  }

  Vec3 operator*(const float s) const { return Vec3(x * s, y * s, z * s); }
  Vec3 operator*(const Vec3& v) const
  {
    return Vec3(x * v.x, y * v.y, z * v.z);
  }

  Vec3 operator+(const Vec3& v) const
  {
    return Vec3(x + v.x, y + v.y, z + v.z);
  }

  Vec3 operator-(const float& v) const { return Vec3(x - v, y - v, z - v); }

  Vec3 operator+(const float& v) const { return Vec3(x + v, y + v, z + v); }
  Vec3 operator/(const float c) const { return Vec3(x / c, y / c, z / c); }
  Vec3 operator/(const Vec3 c) const { return Vec3(x / c.x, y / c.y, z / c.z); }
  friend std::ostream& operator<<(std::ostream& out, const Vec3& v)
  {
    out << "(" << v.x << "," << v.y << "," << v.z << ")";
    return out;
  }

  Vec3 cross(const Vec3& v) const
  {
    return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }

  float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }

  float get(char c)
  {
    if (c == 'x')
      return x;
    if (c == 'y')
      return y;
    if (c == 'z')
      return z;
    return std::numeric_limits<float>::max();
  }

  float get(int c)
  {
    switch (c)
    {
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
      default:
        return std::numeric_limits<float>::max();
    }
  }

  void set(char c, float v)
  {
    if (c == 'x')
      x = v;
    if (c == 'y')
      y = v;
    if (c == 'z')
      z = v;
  }

  void set(int c, float v)
  {
    switch (c)
    {
      case 0:
        x = v;
        break;
      case 1:
        y = v;
        break;
      case 2:
        z = v;
        break;
      default:
        break;
    }
  }

  Vec3 min(const Vec3& other) const
  {
    return {std::min(x, other.x), std::min(y, other.y), std::min(z, other.z)};
  }

  Vec3 max(const Vec3& other) const
  {
    return {std::max(x, other.x), std::max(y, other.y), std::max(z, other.z)};
  }

  void normalize()
  {
    float len = length();
    x /= len;
    y /= len;
    z /= len;
  }

  float length() const { return std::sqrt(x * x + y * y + z * z); }

  static Vec3 normalize(const Vec3& v)
  {
    Vec3 vec = v;
    vec.normalize();
    return vec;
  }

  Vec3 lerp(const Vec3& other, float t) const
  {
    return Vec3(x * (1 - t) + other.x * t, y * (1 - t) + other.y * t,
                z * (1 - t) + other.z * t);
  }

  static Vec3 mix(const Vec3& v0, const Vec3& v1, const float t)
  {
    return v0.lerp(v1, t);
  }

  static Vec3 reflect(const Vec3& v, const Vec3& n)
  {
    return v - n * 2 * v.dot(n);
  }

  static Vec3 refract(const Vec3& uv, const Vec3& n, double etai_over_etat)
  {
    auto cos_theta = fmin((uv * -1).dot(n), 1.0);
    Vec3 r_out_perp = (uv + n * cos_theta) * etai_over_etat;
    Vec3 r_out_parallel = n * -sqrt(fabs(r_out_perp.dot(r_out_perp) * -1 + 1));
    return r_out_perp + r_out_parallel;
  }

  inline static Vec3 random_in_unit_disk()
  {
    while (true)
    {
      auto p = Vec3(random_double(-1, 1), random_double(-1, 1), 0);
      if (p.dot(p) < 1)
        return p;
    }
  }
  static Vec3 random()
  {
    return Vec3(random_double(), random_double(), random_double());
  }
  static Vec3 random(double min, double max)
  {
    return Vec3(random_double(min, max), random_double(min, max),
                random_double(min, max));
  }

  Vec3 clamp() const
  {
    return Vec3{std::min(std::max(0.0, x), 1.0),
                std::min(std::max(0.0, y), 1.0),
                std::min(std::max(0.0, z), 1.0)};
  };

  Vec3 clamp(double min, double max) const
  {
    return Vec3{std::max(min, std::min(max, x)),
                std::max(min, std::min(max, y)),
                std::max(min, std::min(max, z))};
  };
};

class Quaternion
{
  public:
  Quaternion() {};
  Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {};

  Quaternion operator*(const Quaternion& q) const
  {
    return Quaternion(w * q.w - x * q.x - y * q.y - z * q.z,
                      w * q.x + x * q.w + y * q.z - z * q.y,
                      w * q.y - x * q.z + y * q.w + z * q.x,
                      w * q.z + x * q.y - y * q.x + z * q.w);
  }

  void normalize()
  {
    float norm = std::sqrt(w * w + x * x + y * y + z * z);
    w /= norm;
    x /= norm;
    y /= norm;
    z /= norm;
  }
  Quaternion conjugate() const { return {w, -x, -y, -z}; }

  float w, x, y, z;
};

Quaternion rotation_from_to(Vec3 from, Vec3 to);
Vec3 rotate_vector(const Vec3& v, const Quaternion& q);

inline Vec3 random_in_unit_sphere()
{
  while (true)
  {
    auto p = Vec3::random(-1, 1);
    if (p.dot(p) < 1)
    {
      return p;
    }
  }
}

inline Vec3 random_unit_vector()
{
  Vec3 v = random_in_unit_sphere();
  Vec3::normalize(v);
  return v;
}

class Mat4
{
  public:
  float m[4][4];

  Mat4(double d, double d1, double d2, float d3, double d4, double d5,
       double d6, float d7, double d8, double d9, double d10, float d11,
       double d12, double d13, double d14, double d15)
  {
    m[0][0] = d;
    m[0][1] = d1;
    m[0][2] = d2;
    m[0][3] = d3;
    m[1][0] = d4;
    m[1][1] = d5;
    m[1][2] = d6;
    m[1][3] = d7;
    m[2][0] = d8;
    m[2][1] = d9;
    m[2][2] = d10;
    m[2][3] = d11;
    m[3][0] = d12;
    m[3][1] = d13;
    m[3][2] = d14;
    m[3][3] = d15;
  }

  Mat4() {}

  Mat4(float* v)
  {
    for (int i = 0; i < 4; ++i)
    {
      for (int j = 0; j < 4; ++j)
      {
        m[i][j] = v[(i * 4) + j];
      }
    }
  }

  Mat4 operator*(const float& v)
  {
    float mat[16];

    for (int i = 0; i < 4; ++i)
    {
      for (int j = 0; j < 4; ++j)
      {
        mat[i * 4 + j] = m[i][j] * v;
      }
    }
    return {mat};
  }

  Vec3 operator*(Vec3 v)
  {
    Vec3 result;
    for (int i = 0; i < 3; ++i)
    {
      float sum = 0;
      for (int j = 0; j < 3; ++j)
      {

        sum += v.get(i) * m[i][j];
      }
      sum = sum + m[i][3];
      result.set(i, sum);
    }
    return result;
  }

  void set(int x, int y, float val) { m[x][y] = val; }

  Mat4 transpose()
  {
    Mat4 t;
    for (int i = 0; i < 4; ++i)
    {
      for (int j = 0; j < 4; ++j)
      {
        t.set(i, j, m[j][i]);
      }
    }
    return t;
  }

  Vec3 translate() { return Vec3{m[0][3], m[1][3], m[2][3]}; }

  Mat4 inverse()
  {
    Mat4 inv;
    Mat4 r_t = transpose();

    Vec3 inverseTranslation = r_t * -1 * translate();

    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        inv.m[i][j] = r_t.m[i][j];
      }
    }

    inv.m[0][3] = inverseTranslation.x;
    inv.m[1][3] = inverseTranslation.y;
    inv.m[2][3] = inverseTranslation.z;

    inv.m[3][0] = inv.m[3][1] = inv.m[3][2] = 0;
    inv.m[3][3] = 1;

    return inv;
  }
};
