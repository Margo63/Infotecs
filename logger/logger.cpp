#include "logger.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace logger {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : initialized(false), defaultLevel(LogLevel::INFO) {}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

bool Logger::initialize(const std::string& filename, LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex);

    if (initialized) {
        logFile.close();
    }

    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        return false;
    }

    defaultLevel = level;
    initialized = true;
    return true;
}

bool Logger::isInitialized() const {
    return initialized;
}

void Logger::log(const std::string& message, LogLevel level) {
    if (!initialized) {
        return;
    }

    if (static_cast<int>(level) < static_cast<int>(defaultLevel.load())) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);

    if (logFile.is_open()) {
        logFile << "[" << getCurrentTime() << "] "
                << "[" << logLevelToString(level) << "] "
                << message << std::endl;
        logFile.flush();
    }
}

void Logger::log(const std::string& message) {
    log(message, defaultLevel.load());
}

void Logger::setDefaultLevel(LogLevel level) {
    defaultLevel = level;
}

LogLevel Logger::getDefaultLevel() const {
    return defaultLevel.load();
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex);
    if (logFile.is_open()) {
        logFile.flush();
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm_now;
    localtime_r(&time_t_now, &tm_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::INFO:    return "INFO";
        default:                return "UNKNOWN";
    }
}

}