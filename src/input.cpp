#include <input.hpp>

#include <iostream>
#include <string>

JSONParser::JSONParser(const std::string&) {};
JSONParser::JSONParser() {};

int JSONParser::read_file(const std::string& filename)
{
  std::ifstream file(filename);
  if (file.bad())
  {
    std::cout << "Error reading file: " << filename << std::endl;
    return -1;
  }

  if (file.is_open())
  {
    json_data = nlohmann::json::parse(file);
  }
  else
  {
    std::cout << "File not found: " << filename << std::endl;
    return -1;
  }

  file.close();
  return 0;
}

bool JSONParser::contains(const std::string& key) const
{
  return json_data.contains(key);
}

void JSONParser::print() const { std::cout << json_data.dump(4) << std::endl; }
