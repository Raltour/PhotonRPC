#include "logger.h"
#include "config.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/basic_file_sink.h"

void Logger::Init() {
  // // 初始化异步日志的线程池
  // // 参数：队列大小=8192，后台线程数=1
  // spdlog::init_thread_pool(Config::GetInstance().log_queue_size(),
  //                          Config::GetInstance().log_thread_num());
  //
  // // 创建彩色控制台sink
  // // auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  //
  // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
  //     Config::GetInstance().log_file_path(),
  //     Config::GetInstance().log_truncate());
  //
  // file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v (%s:%#)");
  //
  // // 设置日志格式：[时间] [级别] 消息 (文件:行号)
  // // console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v (%s:%#)");
  //
  // // 创建异步logger
  // auto logger = std::make_shared<spdlog::async_logger>(
  //     "photonrpc_logger",
  //     // console_sink,
  //     file_sink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
  //
  // // 设置为默认logger
  // spdlog::set_default_logger(logger);
  //
  // // 设置日志级别
  // spdlog::set_level(static_cast<spdlog::level::level_enum>(
  //     Config::GetInstance().log_level()));
  //
  // // 立即刷新日志（对于错误级别）
  // spdlog::flush_on(spdlog::level::err);
  //

  std::shared_ptr<spdlog::logger> mylogger = spdlog::basic_logger_mt("spdlog", Config::GetInstance().log_file_path());
  // 设置日志格式. 参数含义: [日志标识符] [日期] [日志级别] [线程号] [数据]
  mylogger->set_pattern("[%n][%Y-%m-%d %H:%M:%S.%e] [%l] [%t]  %v");
  // mylogger->set_level(spdlog::level::debug);
  mylogger->set_level(spdlog::level::info);
  spdlog::flush_every(std::chrono::seconds(5)); // 定期刷新日志缓冲区

  spdlog::set_default_logger(mylogger);
}
