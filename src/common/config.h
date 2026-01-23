#ifndef PHOTONRPC_CONFIG_H
#define PHOTONRPC_CONFIG_H

#include <map>
#include <string>

class Config {
 public:
  static Config& GetInstance();

  // Public accessors for specific config
  std::string server_host() const { return GetString("server", "host"); }
  int server_port() const { return GetInt("server", "port"); }

  int log_level() const { return GetInt("log", "level"); }
  int log_queue_size() const { return GetInt("log", "queue_size"); }
  int log_thread_num() const { return GetInt("log", "thread_num"); }
  std::string log_file_path() const { return GetString("log", "file_path"); }
  bool log_truncate() const { return GetString("log", "trucate") == "true"; }

 private:
  Config(const std::string& config_path = "../conf/photonrpc.xml");

  void LoadConfigFile(const std::string& file_path);

  std::string GetString(const std::string& section,
                        const std::string& key) const;
  int GetInt(const std::string& section, const std::string& key) const;

  // Stores all configuration: section -> key -> value
  std::map<std::string, std::map<std::string, std::string>> config_map_;
};

#endif  //PHOTONRPC_CONFIG_H
