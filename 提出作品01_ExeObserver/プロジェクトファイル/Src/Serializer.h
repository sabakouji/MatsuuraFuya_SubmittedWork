#pragma once

#include "../include/nlohmann/json.hpp"
#include "CoreData.h"
#include <memory>
#include <string>


using json = nlohmann::json;

class Serializer {
public:
  // Save
  bool Save(const GraphData &graphData, const std::string &filePath) const;
  // Load
  bool Load(const std::string &filePath, GraphData &graphData);

  std::string PinTypeToString(PinType type) const;
  PinType StringToPinType(const std::string &str) const;

  std::string PropertyTypeToString(PropertyType type) const;
  PropertyType StringToPropertyType(const std::string &str) const;
};
