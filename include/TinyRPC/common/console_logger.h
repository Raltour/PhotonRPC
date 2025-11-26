#ifndef CONSOLE_LOGGER_H
#define CONSOLE_LOGGER_H

#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

// 简单的日志级别
enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR
};

class ConsoleLogger {
public:
  // 单例模式：保证全局只有一个日志管理器
  static ConsoleLogger& getInstance() {
    static ConsoleLogger instance;
    return instance;
  }

  // 禁止拷贝
  ConsoleLogger(const ConsoleLogger&) = delete;

  ConsoleLogger& operator=(const ConsoleLogger&) = delete;

  // 核心打印函数
  void log(LogLevel level, const std::string& message, const char* file,
           int line);

private:
  ConsoleLogger() = default;

  std::mutex mutex_; // 互斥锁，保护 std::cout
};

// 定义宏，自动填入 __FILE__ 和 __LINE__
// 使用 do-while(0) 是为了让宏像函数一样安全使用
#define LOG_DEBUG(msg) ConsoleLogger::getInstance().log(LogLevel::DEBUG, msg, __FILE__, __LINE__)
#define LOG_INFO(msg)  ConsoleLogger::getInstance().log(LogLevel::INFO,  msg, __FILE__, __LINE__)
#define LOG_WARN(msg)  ConsoleLogger::getInstance().log(LogLevel::WARN,  msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) ConsoleLogger::getInstance().log(LogLevel::ERROR, msg, __FILE__, __LINE__)


#endif // CONSOLE_LOGGER_H