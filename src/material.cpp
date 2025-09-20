#include "material.hpp"
#include "image.hpp"
#include "intersection.hpp"
#include "light.hpp"
#include "random.h"
#include "ray.hpp"
#include "vector.hpp"

Material::Material()
    : ks(1.0f), kd(1.0f), specular_exp(1), diffuse_color(PPMColor()),
      specular_color(PPMColor()), is_reflective(false), reflectivity(1.0f),
      is_refractive(true), refractive_index(1.0f),
      texture(std::make_shared<Texture>())
{
}

Material::Material(const Material& m)
    : ks(m.ks), kd(m.kd), specular_exp(m.specular_exp),
      diffuse_color(m.diffuse_color), specular_color(m.specular_color),
      is_reflective(m.is_reflective), reflectivity(m.reflectivity),
      is_refractive(m.is_refractive), refractive_index(m.refractive_index),
      texture(m.texture)
{
}

DiffuseMaterial::DiffuseMaterial(const Material& mat) : Material(mat) {}
bool DiffuseMaterial::scatter(Ray& ray, Intersection& hit_info,
                              Vec3& attenuation, Ray& scattered)
{
  Vec3 scatter_dir = hit_info.normal + random_unit_vector();
  if (scatter_dir.dot(hit_info.normal) < 0)
    scatter_dir * -1;
  scatter_dir.normalize();

  scattered = {hit_info.position, scatter_dir};
  attenuation = diffuse_color.to_vec();
  return true;
}

bool DiffuseMaterial::get_new_ray(Ray& ray, Intersection& hit_info,
                                  Vec3& attenuation, Ray& scattered)
{
  attenuation = diffuse_color.to_vec() * kd;
  return false;
}

ReflectiveMaterial::ReflectiveMaterial(const Material& mat)
    : Material(mat), roughness(0.2f), metallic(1.0f)
{
}

Vec3 ReflectiveMaterial::sample_ggx(const Vec3& normal, float roughness) const
{
  Vec3 rand = Vec3::normalize(Vec3::random());

  float alpha = roughness * roughness;
  float phi = 2.0 * M_PI * rand.x;
  float theta =
      acos(sqrt((1.0f - rand.y) / ((alpha * alpha - 1.0f) * rand.y + 1.0f)));

  Vec3 h;
  h.x = cos(phi) * sin(theta);
  h.y = sin(phi) * sin(theta);
  h.z = cos(theta);

  Vec3 up =
      abs(normal.z) < 0.99 ? Vec3(0.0f, 0.0f, 1.0f) : Vec3(1.0f, 0.0f, 0.0f);
  Vec3 tangent = Vec3::normalize(up.cross(normal));
  Vec3 bitangent = normal.cross(tangent);

  return Vec3::normalize(tangent * h.x + bitangent * h.y + normal * h.z);
}

bool ReflectiveMaterial::scatter(Ray& ray, Intersection& hit_info,
                                 Vec3& attenuation, Ray& scattered)
{
  Vec3 unit_dir = Vec3::normalize(ray.direction) * -1;
  Vec3 microfacet_normal = sample_ggx(hit_info.normal, roughness);
  // std::cout << "Dot: " << microfacet_normal.dot(hit_info.normal) << ",
  // Normal: " << hit_info.normal << ", Microfacet: " << microfacet_normal <<
  // "Roughness: " << roughness << std::endl;

  if (reflectivity > 0.9)
  {
    microfacet_normal = hit_info.normal;
  }

  Vec3 scatter_dir = Vec3::reflect(ray.direction, microfacet_normal);
  scattered = {hit_info.position + scatter_dir * 0.0001f, scatter_dir};
  Vec3 light_dir = scatter_dir;

  attenuation = calculate_color(unit_dir, hit_info.normal, light_dir);
  return true;
}

float ReflectiveMaterial::ggxNDF(const Vec3& half_vector,
                                 const Vec3& normal) const
{
  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float NdotH = std::max(normal.dot(half_vector), 0.0f);
  float denom = (NdotH * NdotH) * (alpha2 - 1.0) + 1.0;
  return alpha2 / (M_PI * denom * denom);
}

// Schlick's Fresnel Approximation
Vec3 ReflectiveMaterial::fresnelSchlick(float cosTheta, const Vec3& F0) const
{
  return F0 + (F0 * -1 + 1) * pow(1.0f - cosTheta, 5.0f);
}

// Smith's Geometry Function
float ReflectiveMaterial::geometrySchlickGGX(float NdotV) const
{
  float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
  return NdotV / (NdotV * (1.0f - k) + k);
}

Vec3 ReflectiveMaterial::calculate_color(Vec3& view_dir, Vec3& normal,
                                         Vec3& scattered)
{
  roughness = std::max(0.05f, 1 - reflectivity);
  Vec3 albedo = diffuse_color.to_vec();
  Vec3 light_dir = Vec3::normalize(scattered);

  Vec3 half_vector = Vec3::normalize(view_dir + light_dir);
  float NdotV = std::max(normal.dot(view_dir), 0.0f);
  float NdotL = std::max(normal.dot(light_dir), 0.0f);
  float HdotV = std::max(half_vector.dot(view_dir), 0.0f);

  float r0 = reflectivity;
  Vec3 F0 = Vec3{r0, r0, r0};
  F0 = Vec3::mix(F0, albedo, 0.1);
  Vec3 F = fresnelSchlick(HdotV, F0);

  float D = ggxNDF(half_vector, normal);
  D = std::max(0.0f, std::min(5.0f, D));
  float G = geometrySchlickGGX(NdotV) * geometrySchlickGGX(NdotL);

  Vec3 nominator = F * D * G;
  float denominator =
      4.0f * NdotV * NdotL + 0.001f; // 0.001 to prevent division by zero

  Vec3 specular = nominator / denominator;

  // Combine specular and diffuse components
  Vec3 kS = F.clamp();
  Vec3 kD = Vec3{1.0, 1.0, 1.0} - kS;

  Vec3 diffuse = kD * albedo / M_PI;

  return (diffuse + specular) * NdotL;
}

bool ReflectiveMaterial::get_new_ray(Ray& ray, Intersection& hit_info,
                                     Vec3& attenuation, Ray& scattered)
{
  return scatter(ray, hit_info, attenuation, scattered);
}

RefractiveMaterial::RefractiveMaterial(const Material& mat) : Material(mat) {}
bool RefractiveMaterial::scatter(Ray& ray, Intersection& hit_info,
                                 Vec3& attenuation, Ray& scattered)
{
  attenuation = Vec3{1.0f, 1.0f, 1.0f};
  Vec3 out_norm = hit_info.normal;
  Vec3 out_dir;
  float ir = 1 / refractive_index;
  if (ray.direction.dot(hit_info.normal) > 0)
  {
    out_norm = hit_info.normal * -1;
    ir = refractive_index;
  }

  double cos_theta = fmin((ray.direction * -1).dot(out_norm), 1.0);
  double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

  bool cannot_refract = ir * sin_theta > 1.0;

  auto r0 = (1 - ir) / (1 + ir);
  r0 *= r0;
  r0 = r0 + (1 - r0) * pow((1 - cos_theta), 5);

  if (cannot_refract || r0 > random_double())
  {
    scattered = {hit_info.position, ray.direction};
    return false;
  }

  out_dir = Vec3::refract(ray.direction, out_norm, ir);
  scattered = {hit_info.position + out_dir * 0.00001f, out_dir};
  return true;
}

bool RefractiveMaterial::get_new_ray(Ray& ray, Intersection& hit_info,
                                     Vec3& attenuation, Ray& scattered)
{
  return scatter(ray, hit_info, attenuation, scattered);
}

RRMaterial::RRMaterial(const Material& mat)
    : Material(mat), ref_mat(mat), rel_mat(mat)
{
}

void RRMaterial::pass_values()
{
  ref_mat.refractive_index = refractive_index;
  ref_mat.reflectivity = reflectivity;

  rel_mat.reflectivity = reflectivity;
  rel_mat.diffuse_color = diffuse_color;
  rel_mat.specular_color = specular_color;
}

bool RRMaterial::scatter(Ray& ray, Intersection& hit_info, Vec3& attenuation,
                         Ray& scattered)
{
  if (!set_mat)
  {
    pass_values();
    set_mat = true;
  }

  if (ref_mat.scatter(ray, hit_info, attenuation, scattered))
  {
    return true;
  }
  return rel_mat.scatter(ray, hit_info, attenuation, scattered);
}

bool RRMaterial::get_new_ray(Ray& ray, Intersection& hit_info,
                             Vec3& attenuation, Ray& scattered)
{
  return scatter(ray, hit_info, attenuation, scattered);
}

EmissiveMaterial::EmissiveMaterial(const Material& mat, const AreaLight& light)
    : Material(mat), light(light)
{
}

bool EmissiveMaterial::scatter(Ray& ray, Intersection& hit_info,
                               Vec3& attenuation, Ray& scattered)
{
  attenuation = light.intensity;
  return false;
}

bool EmissiveMaterial::get_new_ray(Ray& ray, Intersection& hit_info,
                                   Vec3& attenuation, Ray& scattered)
{
  return false;
}
