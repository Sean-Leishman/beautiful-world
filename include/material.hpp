#pragma once

#include "image.hpp"
#include "intersection.hpp"
#include "light.hpp"
#include "ray.hpp"

#include <memory>

class Texture
{
  public:
  PPMImage image;
  bool loaded;

  Texture() : loaded(false) {};
  Texture(const std::string& filename) : loaded(false)
  {
    loaded = image.read_from_file(filename);
  }

  PPMColor get_color(float u, float v) const { return image.get_pixel(u, v); }
};

class Material
{
  public:
  float ks{};
  float kd{};
  int specular_exp{};
  PPMColor diffuse_color;
  PPMColor specular_color;
  bool is_reflective{};
  float reflectivity{};
  bool is_refractive{};
  float refractive_index{};

  std::shared_ptr<Texture> texture;

  Material();
  Material(const Material&);
  Material(float, float, int, PPMColor, PPMColor, bool, float, bool, float,
           std::string);
  virtual ~Material() = default;

  virtual Vec3 emission() const = 0;
  virtual bool scatter(Ray& ray, Intersection& hit_info, Vec3& attenuation,
                       Ray& scattered) = 0;
  virtual bool get_new_ray(Ray&, Intersection&, Vec3&, Ray&) = 0;
};

class DiffuseMaterial : public Material
{
  public:
  DiffuseMaterial() {};
  DiffuseMaterial(const Material& mat);
  bool scatter(Ray& ray, Intersection& hit_info, Vec3& attenuation,
               Ray& scattered) override;
  Vec3 emission() const override { return {}; };
  bool get_new_ray(Ray& ray, Intersection& hit_info, Vec3& attenuation,
                   Ray& scattered) override;
};

class ReflectiveMaterial : public Material
{
  protected:
  float roughness;
  float metallic;

  Vec3 sample_ggx(const Vec3& normal, float roughness) const;
  float ggxNDF(const Vec3& half_vector, const Vec3& normal) const;
  Vec3 fresnelSchlick(float cosTheta, const Vec3& F0) const;
  float geometrySchlickGGX(float NdotV) const;

  public:
  ReflectiveMaterial() : roughness(0.5f), metallic(0.1f) {};
  ReflectiveMaterial(const Material& mat);
  bool scatter(Ray& ray, Intersection& hit_info, Vec3& attenuation,
               Ray& scattered) override;
  Vec3 emission() const override { return {}; };
  bool get_new_ray(Ray& ray, Intersection& hit_info, Vec3& attenuation,
                   Ray& scattered) override;
  Vec3 calculate_color(Vec3&, Vec3&, Vec3&);
};

class RefractiveMaterial : public Material
{
  public:
  RefractiveMaterial() {};
  RefractiveMaterial(const Material& mat);
  bool scatter(Ray& ray, Intersection& hit_info, Vec3& attenuation,
               Ray& scattered) override;

  Vec3 emission() const override { return {}; };
  bool get_new_ray(Ray& ray, Intersection& hit_info, Vec3& attenuation,
                   Ray& scattered) override;
};

class RRMaterial : public Material
{
  private:
  bool set_mat = false;

  public:
  RefractiveMaterial ref_mat;
  ReflectiveMaterial rel_mat;

  void pass_values();

  RRMaterial() {};
  RRMaterial(const Material& mat);
  bool scatter(Ray& ray, Intersection& hit_info, Vec3& attenuation,
               Ray& scattered) override;
  Vec3 emission() const override { return {}; };
  bool get_new_ray(Ray& ray, Intersection& hit_info, Vec3& attenuation,
                   Ray& scattered) override;
};

class EmissiveMaterial : public Material
{
  public:
  AreaLight light;
  EmissiveMaterial() {};
  EmissiveMaterial(const Material&, const AreaLight&);

  bool scatter(Ray& ray, Intersection& hit_info, Vec3& attenuation,
               Ray& scattered) override;
  Vec3 emission() const override { return light.color.to_vec(); };
  bool get_new_ray(Ray& ray, Intersection& hit_info, Vec3& attenuation,
                   Ray& scattered) override;
  Vec3 calculate_color(Ray&, Intersection&, Ray&) const;
};
