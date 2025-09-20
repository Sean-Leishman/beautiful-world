#include "renderer.hpp"
#include "camera.hpp"
#include "input.hpp"
#include "scene.hpp"
#include "shape.hpp"

#include "json.hpp"
#include "light.hpp"
#include "material.hpp"
#include <execution>
#include <filesystem>
#include <memory>

void Renderer::render_frame(std::string save_file)
{
  std::cout << "render" << std::endl;
  scene.build_bvh();
  std::cout << "built tree" << std::endl;

  /*
const int total_pixels = image_width * image_height;
std::cout << "total pixels: " << total_pixels << std::endl;
std::cout << "image size: " << image_width << "x" << image_height
          << std::endl;

std::vector<int> pixel_indicies;
std::iota(pixel_indicies.begin(), pixel_indicies.end(), 0);

for (int i = 0; i < total_pixels; ++i)
{
  std::for_each(std::execution::par_unseq, pixel_indicies.begin(),
                pixel_indicies.end(),
                [&](const auto pixel_idx)
                {
                  int x = pixel_idx % image_width;
                  int y = pixel_idx / image_width;
                  auto color = raytracer->trace_ray((float)x, (float)y);
                  image.set_pixel(x, y, color);
                });
}
*/

  for (int x = 0; x < image_width; ++x)
  {
    for (int y = 0; y < image_height; ++y)
    {
      PPMColor color = raytracer->trace_ray((float)x, (float)y);
      image.set_pixel(x, y, color);
    }
    std::cout << "scan line: " << x << std::endl;
  }
  std::cout << scene.shapes[0] << "\n";

  std::filesystem::path path(std::filesystem::current_path());
  // path += "/materials/2.ppm";
  path += save_file;
  image.save_to_file(path);
}

std::unique_ptr<Material> Renderer::load_material(nlohmann::json j)
{

  bool is_reflective = j["isreflective"];
  bool is_refractive = j["isrefractive"];

  std::shared_ptr<Texture> texture;

  if (j.contains("texture"))
  {
    std::string key = j["texture"];
    if (textures.find(key) == textures.end())
    {
      std::filesystem::path filename(std::filesystem::current_path());
      filename += key;
      auto ptr = std::make_shared<Texture>(Texture{filename});

      textures.insert(std::make_pair(key, std::move(ptr)));
    }
    auto texture_ptr = textures.find(key);
    texture = texture_ptr->second;
  }
  else
  {
    texture = std::make_shared<Texture>();
  }

  std::unique_ptr<Material> material;
  if (is_reflective && is_refractive)
  {
    material = std::make_unique<RRMaterial>();
  }
  else if (is_reflective)
  {
    material = std::make_unique<ReflectiveMaterial>();
  }
  else if (is_refractive)
  {
    material = std::make_unique<RefractiveMaterial>();
  }
  else
  {
    material = std::make_unique<DiffuseMaterial>();
  }

  material->ks = j["ks"];
  material->kd = j["kd"];
  material->specular_exp = j["specularexponent"];
  material->diffuse_color = PPMColor(j["diffusecolor"]);
  material->specular_color = PPMColor(j["specularcolor"]);
  material->refractive_index = j["refractiveindex"];
  material->reflectivity = j["reflectivity"];
  material->texture = std::move(texture);
  material->is_reflective = is_reflective;
  material->is_refractive = is_refractive;

  return material;
}

void Renderer::load_lights(nlohmann::json lights)
{
  scene.ambient_light = std::make_shared<AmbientLight>(Vec3(0.0f, 0.0f, 0.0f),
                                                       Vec3(0.0f, 0.0f, 0.0f));
  for (const auto& light : lights)
  {
    std::string type = light["type"];
    std::shared_ptr<Light> new_light;
    std::shared_ptr<Shape> new_shape;

    if (type == "pointlight")
    {
      std::vector<float> position = light["position"];
      std::vector<float> intensity = light["intensity"];

      new_light = std::make_shared<PointLight>(position, intensity);
      scene.lights.push_back(new_light);
    }
    else if (type == "ambientlight")
    {
      std::vector<float> intensity = light["intensity"];
      PPMColor color{light["color"]};
      new_light = std::make_shared<AmbientLight>(intensity, color);
      scene.ambient_light = new_light;
    }
    else if (type == "arealight")
    {
      std::vector<float> position = light["position"];
      std::vector<float> intensity = light["intensity"];
      std::vector<float> size = light["size"];
      std::vector<float> normal = light["normal"];

      auto light = AreaLight(position, intensity, normal, size);
      new_light = std::make_shared<AreaLight>(light);

      if (input_render == "pathtracer")
      {
        std::unique_ptr<Material> mat =
            std::make_unique<EmissiveMaterial>(DiffuseMaterial(), light);
        new_shape = std::make_shared<Sphere>(Vec3{position}, size[0] * 0.1,
                                             std::move(mat));
        scene.shapes.push_back(new_shape);
      }
      else
      {
        scene.lights.push_back(new_light);
      }
    }
  }
}

void Renderer::load_shapes(nlohmann::json shapes)
{
  for (const auto& shape : shapes)
  {
    std::string type = shape["type"];
    std::shared_ptr<Shape> new_shape;
    if (type == "sphere")
    {
      std::vector<float> center = shape["center"];
      float radius = shape["radius"];

      std::unique_ptr<Material> material;
      if (shape.contains("material"))
      {
        material = load_material(shape["material"]);
      }

      new_shape =
          std::make_shared<Sphere>(Vec3{center}, radius, std::move(material));
    }
    else if (type == "cylinder")
    {
      std::vector<float> center = shape["center"];
      std::vector<float> axis = shape["axis"];
      float radius = shape["radius"];
      float height = shape["height"];

      std::unique_ptr<Material> material;
      if (shape.contains("material"))
      {
        material = load_material(shape["material"]);
      }
      new_shape = std::make_shared<Cylinder>(Vec3{center}, Vec3{axis}, radius,
                                             height, std::move(material));
    }
    else if (type == "triangle")
    {
      std::vector<float> v0 = shape["v0"];
      std::vector<float> v1 = shape["v1"];
      std::vector<float> v2 = shape["v2"];

      std::unique_ptr<Material> material;
      if (shape.contains("material"))
      {
        material = load_material(shape["material"]);
      }
      new_shape = std::make_shared<Triangle>(Vec3{v0}, Vec3{v1}, Vec3{v2},
                                             std::move(material));
    }

    if (new_shape != nullptr)
    {
      scene.shapes.push_back(new_shape);
    }
  }
}

int Renderer::load_file(const std::string& filename)
{
  std::filesystem::path path(std::filesystem::current_path());
  path += filename;
  parser.read_file(path.string());

  input_render = parser.get<std::string>("rendermode");

  image_width = parser.get<int>("camera", "width");
  image_height = parser.get<int>("camera", "height");

  image.set(image_width, image_height, 255);

  auto position = parser.get<std::vector<float>>("camera", "position");
  auto lookAt = parser.get<std::vector<float>>("camera", "lookAt");
  auto upVector = parser.get<std::vector<float>>("camera", "upVector");
  auto fov = parser.get<float>("camera", "fov");
  auto exposure = parser.get<float>("camera", "exposure");

  camera =
      PRenderHole{image_width,    image_height, Vec3(position), Vec3(lookAt),
                  Vec3(upVector), fov,          exposure};

  auto scene_json = parser.get<nlohmann::json>("scene");
  auto bg = scene_json["backgroundcolor"];
  scene.bg_color = PPMColor{bg};

  auto shapes = parser.get<std::vector<nlohmann::json>>("scene", "shapes");
  load_shapes(shapes);

  if (scene_json.contains("lightsources"))
  {
    load_lights(
        parser.get<std::vector<nlohmann::json>>("scene", "lightsources"));
  }

  if (input_render == std::string("binary"))
  {
    raytracer = std::make_unique<BinaryRaytracer>(&scene, &camera);
  }
  else if (input_render == std::string("phong"))
  {
    nbounces = parser.get<int>("nbounces");
    raytracer = std::make_unique<PhongRaytracer>(&scene, &camera, nbounces);
  }
  else if (input_render == std::string("pathtracer"))
  {
    nbounces = parser.get<int>("nbounces");
    int nsamples = parser.get<int>("nsamples");
    raytracer =
        std::make_unique<Pathtracer>(&scene, &camera, nbounces, nsamples);
    camera.defocus = true;
  }
  else
  {
    throw std::runtime_error("Raytracer not initialised");
  }

  // scene.apply_transform(camera.transform_matrix);

  return 0;
}

Renderer::Renderer() {}
