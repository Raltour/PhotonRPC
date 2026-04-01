#include "logger.h"
#include "config.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void Logger::Init() {
  // Use reasonable defaults if config fails
  int queue_size = Config::GetInstance().log_queue_size();
  if (queue_size <= 0) queue_size = 8192;
  
  int thread_num = Config::GetInstance().log_thread_num();
  if (thread_num <= 0) thread_num = 1;

  std::string file_path = Config::GetInstance().log_file_path();
  if (file_path.empty()) file_path = "photonrpc.log";

  // Initialize thread pool for async logging
  spdlog::init_thread_pool(queue_size, thread_num);

  // Create basic file sink
  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      file_path, Config::GetInstance().log_truncate());

  // Set the format pattern: [time] [level] [thread] [file:line] message
  file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");

  // Create an async logger
  auto logger = std::make_shared<spdlog::async_logger>(
      "photonrpc", file_sink, spdlog::thread_pool(),
      spdlog::async_overflow_policy::block);

  // Set as the default logger
  spdlog::set_default_logger(logger);

  // Set the active log level
  spdlog::set_level(static_cast<spdlog::level::level_enum>(
      Config::GetInstance().log_level()));

  // Flush buffer to file periodically
  spdlog::flush_every(std::chrono::seconds(3));
}
