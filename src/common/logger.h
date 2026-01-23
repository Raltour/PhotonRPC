#ifndef PHOTONRPC_LOGGER_H
#define PHOTONRPC_LOGGER_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

class Logger {
 public:
  // 单例模式：保证全局只有一个日志管理器
  // static Logger& GetInstance() {
  //   static Logger instance;
  //   return instance;
  // }

  // 禁止拷贝
  // Logger(const Logger&) = delete;
  // Logger& operator=(const Logger&) = delete;

  static void Init();

  // // 设置日志级别
  // static void SetLogLevel(spdlog::level::level_enum level) {
  //   spdlog::set_level(level);
  // }
  //
  // // 获取底层spdlog logger
  // static std::shared_ptr<spdlog::logger> GetLogger() {
  //   return spdlog::default_logger();
  // }

 private:
  // Logger();
  // ~Logger() { spdlog::shutdown(); }
};

// 定义宏，自动填入文件名和行号
// 使用spdlog的格式化日志宏
// #define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
// #define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
// #define LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
// #define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)

#endif  // PHOTONRPC_LOGGER_H
