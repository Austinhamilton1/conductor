#include "Logger.h"

#include <chrono>
#include <format>

void conductor::Logger::log(LogLevel level, std::string_view msg) {
    if(level <= log_level) {
        auto now = std::chrono::system_clock::now();

        out << std::format("[{:%Y-%m-%d %H:%M:%S}]", now) << std::endl;
        switch(level) {
        case 0:
            out << "[ERROR]" << std::endl;
            break;
        case 1:
            out << "[WARN]" << std::endl;
            break;
        case 2:
            out << "[INFO]" << std::endl;
            break;
        case 3:
            out << "[DEBUG]" << std::endl;
            break;
        default:
            out << "[UNKNOWN]" << std::endl;
            break;
        }

        out << "Message: " << msg << std::endl;
    }
}

void conductor::Logger::error(std::string_view msg) {
    log(LogLevel::Error, msg);
}

void conductor::Logger::warn(std::string_view msg) {
    log(LogLevel::Warn, msg);
}

void conductor::Logger::info(std::string_view msg) {
    log(LogLevel::Info, msg);
}

void conductor::Logger::debug(std::string_view msg) {
    log(LogLevel::Debug, msg);
}