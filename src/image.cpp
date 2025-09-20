#include "image.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

PPMImage::PPMImage(int width, int height, int max_color_value)
    : width(width), height(height), max_color_value(max_color_value)
{
  data.resize(width * height, PPMColor{});
}

void PPMImage::set_pixel(int x, int y, PPMColor color)
{
  if (x < 0 || x >= width || y < 0 || y >= height)
    return;
  data[(y * width) + x] = color;
}

PPMColor PPMImage::get_pixel(float u, float v) const
{
  int x = u * width;
  int y = v * height;

  if ((y * width) + x >= static_cast<int>(data.size()))
  {
    return PPMColor();
  }

  return data[(y * width) + x];
}

bool PPMImage::save_to_file(const std::string& filename)
{
  std::ofstream image_file(filename);
  if (!image_file.is_open())
  {
    std::cerr << "failed to open file for saving: " << filename << std::endl;
    return false;
  }

  // Write header
  image_file << "P3\n";
  image_file << width << " " << height << "\n";
  image_file << max_color_value << "\n";

  // Write pixel data
  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      image_file << data[(y * width) + x] << "\n";
    }
  }

  image_file.close();
  return true;
}

bool PPMImage::read_from_file(const std::string& filename)
{
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open())
  {
    std::cerr << "failed to open file for reading: " << filename << std::endl;
    return false;
  }

  std::string line;
  std::getline(file, line);
  bool p6 = false;
  if (line == "P6")
  {
    p6 = true;
  }

  // Skip comments
  while (file.peek() == '#')
  {
    std::getline(file, line);
  }
  // Read image dimensions
  file >> width >> height;

  // Read max color value (assuming it's 255)
  float maxVal;
  file >> maxVal;

  // Skip the remaining whitespace
  file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // Read pixel data
  data.resize(height * width);
  std::vector<unsigned char> out(width * height * 3);
  if (p6)
  {
    file.read(reinterpret_cast<char*>(out.data()), out.size());
  }

  for (int y = 0; y < height; ++y)
  {
    if (p6)
    {
      for (int x = 0; x < width; ++x)
      {
        int index = (y * width + x) * 3;
        PPMColor color{static_cast<float>(out[index]) / maxVal,
                       static_cast<float>(out[index + 1]) / maxVal,
                       static_cast<float>(out[index + 2]) / maxVal};

        data[y * width + x] = color;
      }
    }
    else
    {
      for (int x = 0; x < width; ++x)
      {
        float r, g, b;
        file >> r >> g >> b;
        PPMColor color{r / static_cast<float>(max_color_value),
                       g / static_cast<float>(max_color_value),
                       b / static_cast<float>(max_color_value)};
        data[y * width + x] = color;
      }
    }
  }

  if (!file)
  {
    std::cerr << "Error occurred while reading the PPM file" << std::endl;
    return false;
  }

  file.close();

  std::filesystem::path path(std::filesystem::current_path());
  path += "/materials/" + filename.substr(filename.find(".") - 3);

  std::cout << "save file: " << filename << ", to: " << path;
  save_to_file(path);
  return true;
}
