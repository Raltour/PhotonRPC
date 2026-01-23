#include "config.h"
#include <iostream>
#include <memory>
#include "common/logger.h"
#include "tinyxml2/tinyxml2.h"

Config& Config::GetInstance() {
  static Config config("../conf/photonrpc.xml");
  return config;
}

Config::Config(const std::string& config_path) {
  LoadConfigFile(config_path);
}

void Config::LoadConfigFile(const std::string& file_path) {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError error = doc.LoadFile(file_path.c_str());
  if (error != tinyxml2::XML_SUCCESS) {
    // LOG_ERROR("Error: " + std::string(doc.ErrorStr()));
    return;
  }

  tinyxml2::XMLElement* root = doc.FirstChildElement("root");
  if (!root) {
    // LOG_ERROR("No <root>.\n");
    return;
  }

  // Generic parsing: iterate over all children of root (sections)
  tinyxml2::XMLElement* element = root->FirstChildElement();
  while (element) {
    std::string section = element->Name();
    const tinyxml2::XMLAttribute* attr = element->FirstAttribute();
    // Iterate over all attributes in the section
    while (attr) {
      config_map_[section][attr->Name()] = attr->Value();
      attr = attr->Next();
    }
    element = element->NextSiblingElement();
  }
}

std::string Config::GetString(const std::string& section,
                              const std::string& key) const {
  if (config_map_.contains(section) && config_map_.at(section).contains(key)) {
    return config_map_.at(section).at(key);
  }
  return "";
}

int Config::GetInt(const std::string& section, const std::string& key) const {
  std::string val = GetString(section, key);
  if (val.empty())
    return 0;
  try {
    return std::stoi(val);
  } catch (...) {
    return 0;
  }
}
