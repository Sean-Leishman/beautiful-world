#pragma once

#include <random>

inline double random_double() { return rand() / (RAND_MAX + 1.0); }

inline double random_double(double min, double max)
{
  // Returns a random real in [min,max).
  return min + (max - min) * random_double();
}
