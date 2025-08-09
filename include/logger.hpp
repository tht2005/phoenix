#ifndef __LOGGER_HPP
#define __LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace logger {

    void init(const std::string& log_level);

    int init_file_logger(const std::string& file_path, const std::string& log_level);

}

std::shared_ptr<spdlog::logger> log();

#endif
