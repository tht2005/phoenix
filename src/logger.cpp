#include "logger.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace logger {

    std::shared_ptr<spdlog::logger> logobj = NULL;

    void init(const std::string& log_level) {
        logobj = spdlog::stderr_color_mt("stderr");
        logobj->set_level(spdlog::level::from_str(log_level));
    }

    int init_file_logger(const std::string& file_path, const std::string& log_level) {
        try {
            logobj = spdlog::basic_logger_mt("logfile", file_path);
            logobj->set_level(spdlog::level::from_str(log_level));
            return 0;
        }
        catch (const spdlog::spdlog_ex& ex) {
            SPDLOG_CRITICAL("Log init failed: {}", ex.what());
            exit (1);
        }
    }

}

std::shared_ptr<spdlog::logger> log() {
    if (!logger::logobj) {
        SPDLOG_CRITICAL("Logger haven't been initialized yet!");
        exit (1);
    }
    return logger::logobj;
}

