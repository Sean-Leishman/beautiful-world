#include "renderer.hpp"
#include <iostream>
#include <omp.h>
#include <string>

int main(int argc, char* argv[])
{
  std::cout << "OpenMP threads available: " << omp_get_max_threads()
            << std::endl;

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
