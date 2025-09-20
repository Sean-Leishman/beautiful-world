#pragma once

#include "input.hpp"

#include "camera.hpp"
#include "image.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "raytracer.hpp"
#include "scene.hpp"

#include <unordered_map>
#include <memory>

enum RenderMode
{
  BINARY,
  PHONG,
};

class Renderer
{
  private:
  int image_width;
  int image_height;
  int nbounces;
  std::string input_render;

  PRenderHole camera;
  std::unique_ptr<Raytracer> raytracer;
  Scene scene;
  PPMImage image;
  std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

  JSONParser parser;

  std::unique_ptr<Material> load_material(nlohmann::json j);
  void load_shapes(nlohmann::json j);
  void load_lights(nlohmann::json j);

  public:
  ~Renderer() = default;
  Renderer();
  int load_file(const std::string&);

  void render_frame(std::string save_file);
};
