#include "TinyRPC/common/config.h"
#include "TinyRPC/common/console_logger.h"

#include "tinyxml2/tinyxml2.h"

#include <iostream>


Config& Config::GetInstance() {
  static Config config;
  return config;
}

Config::Config() {
  tinyxml2::XMLDocument doc;

  tinyxml2::XMLError error = doc.LoadFile("../conf/tinyrpc.xml");
  if (error != tinyxml2::XML_SUCCESS) {
    std::cout << "Error: " << doc.ErrorStr() << std::endl;
    return;
  }

  tinyxml2::XMLElement* root = doc.FirstChildElement("root");
  if (!root) {
    std::cout << "No <root>.\n";
    return;
  }

  tinyxml2::XMLElement* server = root->FirstChildElement("server");
  if (server) {
    server_host_ = server->Attribute("host");
    server_port_ = server->IntAttribute("port");

    LOG_INFO("Server host : " + server_host_);
    LOG_INFO("Server port : " + std::to_string(server_port_));
  }
}