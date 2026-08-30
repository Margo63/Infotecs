#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <atomic>

namespace logger {

    enum class LogLevel {
        ERROR = 0,
        WARNING = 1,
        INFO = 2
    };

    class Logger {
    public:
        static Logger& getInstance();

        bool initialize(const std::string& filename, LogLevel defaultLevel = LogLevel::INFO);
        bool isInitialized() const;

        void log(const std::string& message, LogLevel level);
        void log(const std::string& message);

        void setDefaultLevel(LogLevel level);
        LogLevel getDefaultLevel() const;

        void flush();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

    private:
        Logger();
        ~Logger();

        static std::string getCurrentTime();
        static std::string logLevelToString(LogLevel level);

        std::ofstream logFile;
        std::mutex mutex;
        std::atomic<LogLevel> defaultLevel;
        bool initialized;
    };

}

#endif