#include "TinyRPC/common/console_logger.h"

void ConsoleLogger::log(LogLevel level, const std::string& message,
                        const char* file, int line) {
  // 获取当前时间
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

  // 核心：加锁，防止多线程输出乱码
  // std::lock_guard 在作用域结束时自动解锁
  std::lock_guard<std::mutex> lock(mutex_);

  // 1. 打印时间 [HH:MM:SS.ms]
  std::cout << "[" << std::put_time(std::localtime(&time_t_now), "%T")
      << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";

  // 2. 打印带颜色的级别标签
  switch (level) {
    case LogLevel::DEBUG:
      std::cout << "\033[36m[DEBUG]\033[0m "; // 青色
      break;
    case LogLevel::INFO:
      std::cout << "\033[32m[INFO] \033[0m "; // 绿色
      break;
    case LogLevel::WARN:
      std::cout << "\033[33m[WARN] \033[0m "; // 黄色
      break;
    case LogLevel::ERROR:
      std::cout << "\033[31m[ERROR]\033[0m "; // 红色
      break;
  }

  // 3. 打印具体消息
  std::cout << message;

  // 4. 打印文件位置 (灰色显示，避免喧宾夺主)
  std::cout << " \033[90m(" << file << ":" << line << ")\033[0m" << std::endl;
}