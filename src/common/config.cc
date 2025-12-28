#include "photonrpc/common/config.h"
#include "photonrpc/common/console_logger.h"

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
    LOG_ERROR("Error: " + std::string(doc.ErrorStr()));
    return;
  }

  tinyxml2::XMLElement* root = doc.FirstChildElement("root");
  if (!root) {
    LOG_ERROR("No <root>.\n");
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