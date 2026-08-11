#pragma once

#include <string_view>
#include <iostream>

namespace conductor {
    enum LogLevel {
        Error = 0,
        Warn = 1,
        Info = 2,
        Debug = 3,
    };

    class Logger {
    public:
        Logger(int max_level, std::ostream& out)
            : log_level(max_level), 
              out(out) {};

        void error(std::string_view msg);
        void warn(std::string_view msg);
        void info(std::string_view msg);
        void debug(std::string_view msg);
        
    private:
        int log_level;
        std::ostream& out;

        void log(LogLevel level, std::string_view msg);
    };
}
