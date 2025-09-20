#include "renderer.hpp"

int debug()
{
  int width = 600;
  int height = 400;

  std::vector<std::vector<PPMColor>> data;

  data.resize(height, std::vector<PPMColor>(width, PPMColor()));

  for (int i = 0; i < width; ++i)
  {
    for (int j = 0; j < height; ++j)
    {
      data[j][i] = PPMColor(50, 50, 50);
    }
  }

  Renderer renderer;

  return 0;
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " <input_file> <output_image>\n";
    return 1;
  }

  std::string filename(argv[1]);
  Renderer renderer;
  renderer.load_file(filename);
  renderer.render_frame(std::string{argv[2]});

  return 0;
}
