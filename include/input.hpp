#pragma once

#include <json.hpp>

#include <fstream>
#include <iostream>
#include <string>

class JSONParser
{
  private:
  nlohmann::json json_data;

  public:
  JSONParser();
  JSONParser(const std::string& filename);

  template <typename T>
  T get(const std::string& key) const
  {
    if (json_data.contains(key))
    {
      return json_data.at(key).get<T>();
    }
    throw std::runtime_error("Key not found");
  };

  template <typename T>
  T getImpl(const nlohmann::json& json)
  {
    return json;
  }

  template <typename T, typename... Keys>
  T getImpl(const nlohmann::json& json, const std::string& key1, Keys... keys)
  {
    std::cout << key1 << "\n";
    const auto& next_json = json.at(key1);
    return getImpl<T>(next_json, keys...);
  }

  template <typename T, typename... Keys>
  T get(Keys... keys)
  {
    return getImpl<T>(json_data, keys...);
  }

  bool contains(const std::string& key) const;

  void print() const;

  int read_file(const std::string& filename);
};
