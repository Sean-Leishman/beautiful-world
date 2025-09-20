#include "raytracer.hpp"
#include "image.hpp"
#include "ray.hpp"
#include "renderer.hpp"
#include "scene.hpp"

PPMColor BinaryRaytracer::trace_ray(float x, float y)
{
  Ray ray = camera->compute_ray(x, y);
  Intersection hit_info;
  PPMColor color;

  if (!scene->intersect(ray, hit_info))
  {
    color = scene->bg_color;
    return color;
  }
  color = PPMColor(0, 1, 0);
  return color;
}

PPMColor PhongRaytracer::trace_ray(float x, float y)
{
  Ray ray = camera->compute_ray(x, y);
  Intersection hit_info;
  PPMColor final{trace_ray(ray, 0, hit_info)};
  final.clamp();
  // final = final.gamma_correct();
  return final;
}

Vec3 PhongRaytracer::trace_ray(Ray& ray, int depth, Intersection& hit_info)
{
  if (depth >= max_depth)
  {
    return Vec3();
  }
  Vec3 color;

  if (!scene->intersect(ray, hit_info))
  {
    return scene->bg_color.to_vec();
  }

  if (hit_info.object->material->is_reflective &&
      !hit_info.object->material->is_refractive)
  {
    Ray reflect_ray = calculate_reflection_ray(ray, hit_info);
    color = color * (1 - hit_info.object->material->reflectivity) +
            trace_ray(reflect_ray, depth + 1, hit_info);
    return color;
  }

  if (hit_info.object->material->is_refractive &&
      !hit_info.object->material->is_reflective)
  {
    double refraction_ratio =
        hit_info.first_hit ? hit_info.object->material->refractive_index
                           : (1 / hit_info.object->material->refractive_index);
    Ray refract_ray = calculate_refraction_ray(ray, hit_info, refraction_ratio);
    color = color + trace_ray(refract_ray, depth + 1, hit_info);
    return color;
  }

  if (hit_info.object->material->is_reflective &&
      hit_info.object->material->is_refractive)
  {
    Vec3 out_norm = hit_info.normal;
    Vec3 out_dir;
    float refraction_ratio = 1 / hit_info.object->material->refractive_index;
    if (ray.direction.dot(hit_info.normal) < 0)
    {
      out_norm = hit_info.normal * -1;
      refraction_ratio = hit_info.object->material->refractive_index;
    }

    double cos_theta = fmin((ray.direction * -1).dot(out_norm), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = refraction_ratio * sin_theta > 1.0;

    if (!cannot_refract)
    {
      Ray refract_ray =
          calculate_refraction_ray(ray, hit_info, refraction_ratio);
      color = color + trace_ray(refract_ray, depth + 1, hit_info);
      return color;
    }
    else
    {
      Ray reflect_ray = calculate_reflection_ray(ray, hit_info);
      color = color * (1 - hit_info.object->material->reflectivity) +
              trace_ray(reflect_ray, depth + 1, hit_info);
      return color;
    }
  }

  Vec3 ambient = hit_info.object->material->diffuse_color.to_vec() *
                 scene->ambient_light->intensity *
                 scene->ambient_light->color.to_vec();
  Vec3 direct = calculate_direct(hit_info);

  color = color + ambient + direct;

  return color;
}

Vec3 PhongRaytracer::calculate_direct(Intersection& hit_info)
{

  Vec3 view_dir = Vec3::normalize(camera->get_position() - hit_info.position);
  Vec3 color;

  if (hit_info.object->material->texture->loaded)
  {
    Vec2 uv = hit_info.object->interpolate_uv(&hit_info);
    color = hit_info.object->material->texture->get_color(uv.u, uv.v).to_vec();
  }

  for (const auto& light : scene->lights)
  {
    Vec3 light_dir = Vec3::normalize(light->position - hit_info.position);
    Ray shadow_ray{hit_info.position, light_dir};
    Intersection shadow_hit;

    if (scene->object_in_shadow(shadow_ray, hit_info.object, &shadow_hit))
    {
      continue;
    }

    Vec3 H = Vec3::normalize(light_dir + view_dir);

    float NdotL = std::max(hit_info.normal.dot(light_dir), 0.0f);
    Vec3 diffuse = hit_info.object->material->diffuse_color.to_vec() *
                   light->intensity * hit_info.object->material->kd * NdotL;

    float NdotH = std::max(hit_info.normal.dot(H), 0.0f);
    Vec3 specular = hit_info.object->material->specular_color.to_vec() *
                    light->intensity * hit_info.object->material->ks *
                    std::pow(NdotH, hit_info.object->material->specular_exp);

    color = color + (diffuse + specular);
  }
  return color;
}

Ray PhongRaytracer::calculate_refraction_ray(Ray& ray, Intersection& hit_info,
                                             double ir)
{
  bool is_front_face = false;
  // dependent on normal of intersection
  if (ray.direction.dot(hit_info.normal) < 0)
  {
    ir = 1 / ir;
    is_front_face = true;
  }

  Vec3 unit_dir = Vec3::normalize(ray.direction);
  Vec3 dir;
  if (!is_front_face)
  {
    // Intersecting with inside of object so take that normal
    dir = Vec3::refract(unit_dir, hit_info.normal * -1, ir);
  }
  else
  {
    dir = Vec3::refract(unit_dir, hit_info.normal, ir);
  }

  unit_dir = Vec3::normalize(dir);
  return {hit_info.position + unit_dir * 0.000001f, unit_dir};
}

Ray PhongRaytracer::calculate_reflection_ray(Ray& ray, Intersection& hit_info)
{
  Vec3 unit_dir = Vec3::normalize(ray.direction);
  Vec3 reflect_dir = Vec3::reflect(unit_dir, hit_info.normal);
  // reflect() preserves length, so already normalized
  return Ray(hit_info.position + reflect_dir * 0.000001f, reflect_dir);
}

PPMColor Pathtracer::trace_ray(float x, float y)
{
  float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
#pragma omp parallel for reduction(+ : sum_x, sum_y, sum_z)
  for (int i = 0; i < n_samples; i++)
  {
    Ray ray = camera->compute_ray(x, y);
    Vec3 color{trace_ray(ray, 0)};
    sum_x += color.x;
    sum_y += color.y;
    sum_z += color.z;
  }
  Vec3 final_color{sum_x / n_samples, sum_y / n_samples, sum_z / n_samples};
  PPMColor color{final_color};
  color.clamp();
  return color;
}

Vec3 Pathtracer::trace_ray(Ray& ray, int depth)
{
  if (depth >= max_depth)
    return Vec3{0.0f, 0.0f, 0.0f};

  Intersection hit_info;
  if (!scene->intersect_bvh(ray, hit_info))
    return scene->bg_color.to_vec();

  Vec3 attenuation;
  Ray scattered;

  if (hit_info.object->material->scatter(ray, hit_info, attenuation, scattered))
  {
    Vec3 indirect_light = trace_ray(scattered, depth + 1);
    return attenuation * indirect_light;
  }

  return attenuation;
}