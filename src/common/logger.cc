#include "photonrpc/common/logger.h"
#include "photonrpc/common/config.h"

Logger::Logger() {
  // 初始化异步日志的线程池
  // 参数：队列大小=8192，后台线程数=1
  spdlog::init_thread_pool(Config::GetInstance().config_queue_size(), Config::GetInstance().config_thread_num());

  // 创建彩色控制台sink
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  // 设置日志格式：[时间] [级别] 消息 (文件:行号)
  console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v (%s:%#)");

  // 创建异步logger
  auto logger = std::make_shared<spdlog::async_logger>(
      "photonrpc_logger", console_sink, spdlog::thread_pool(),
      spdlog::async_overflow_policy::block);

  // 设置为默认logger
  spdlog::set_default_logger(logger);

  // 设置日志级别
  spdlog::set_level(static_cast<spdlog::level::level_enum>(Config::GetInstance().config_level()));

  // 立即刷新日志（对于错误级别）
  spdlog::flush_on(spdlog::level::err);
}
