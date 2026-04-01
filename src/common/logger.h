#ifndef PHOTONRPC_LOGGER_H
#define PHOTONRPC_LOGGER_H

// Set minimum active log level to DEBUG so that SPDLOG_DEBUG works.
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

class Logger {
 public:
  static void Init();
};

// Use spdlog macros to capture filename and line number automatically
#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)  SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)  SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define LOG_FATAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

#endif  // PHOTONRPC_LOGGER_H
