#ifndef PHOTONRPC_CONFIG_H
#define PHOTONRPC_CONFIG_H

#include <string>
#include <map>

class Config {
 public:
  static Config& GetInstance();

  // Public accessors for specific config
  std::string server_host() const { return GetString("server", "host"); }
  int server_port() const { return GetInt("server", "port"); }

  int config_level() const { return GetInt("config", "level"); }
  int config_queue_size() const { return GetInt("config", "queue_size"); }
  int config_thread_num() const { return GetInt("config", "thread_num"); }

 private:
  Config(const std::string& config_path = "../conf/photonrpc.xml");
  
  void LoadConfigFile(const std::string& file_path);

  std::string GetString (const std::string& section, const std::string& key) const;
  int GetInt (const std::string& section, const std::string& key) const;
  
  // Stores all configuration: section -> key -> value
  std::map<std::string, std::map<std::string, std::string>> config_map_;
};

#endif  //PHOTONRPC_CONFIG_H
