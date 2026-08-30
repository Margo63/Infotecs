#include "../logger/logger.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <condition_variable>
#include <deque>

using namespace logger;

class LogWorker {
public:
    LogWorker() : running(true) {}

    ~LogWorker() {
        stop();
    }

    void start() {
        workerThread = std::thread(&LogWorker::processQueue, this);
    }

    void enqueueLog(const std::string& message, LogLevel level) {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.push_back({message, level});
        queueCond.notify_one();
    }

    void stop() {
        running = false;
        queueCond.notify_all();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

private:
    struct LogEntry {
        std::string message;
        LogLevel level;
    };

    void processQueue() {
        while (running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCond.wait(lock, [this] {
                return !logQueue.empty() || !running;
            });

            while (!logQueue.empty()) {
                auto entry = logQueue.front();
                logQueue.pop_front();
                lock.unlock();

                Logger::getInstance().log(entry.message, entry.level);

                lock.lock();
            }
        }
    }

    std::thread workerThread;
    std::atomic<bool> running;
    std::mutex queueMutex;
    std::condition_variable queueCond;
    std::deque<LogEntry> logQueue;
};

LogLevel parseLogLevel(const std::string& str) {
    if (str == "ERROR" || str == "error" || str == "0") {
        return LogLevel::ERROR;
    } else if (str == "WARNING" || str == "warning" || str == "1") {
        return LogLevel::WARNING;
    } else if (str == "INFO" || str == "info" || str == "2") {
        return LogLevel::INFO;
    }
    return LogLevel::INFO;
}

void info() {
    std::cout << "\nCommands:" << std::endl;
    std::cout << "  <message> \t\t- log with default level\n";
    std::cout << "  <message> <level> \t\t- log with specified level\n";
    std::cout << "  set_level <level> \t\t- change default level\n";
    std::cout << "  help \t\t- show this help\n";
    std::cout << "  quit \t\t- exit program\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <log_file> [default_level]" << std::endl;
        std::cerr << "Default levels: ERROR(0), WARNING(1), INFO(2)" << std::endl;
        return 1;
    }

    std::string logFilename = argv[1];
    LogLevel defaultLevel = LogLevel::INFO;

    if (argc >= 3) {
        defaultLevel = parseLogLevel(argv[2]);
    }

    if (!Logger::getInstance().initialize(logFilename, defaultLevel)) {
        std::cerr << "Failed to initialize logger with file: "
                  << logFilename << std::endl;
        return 1;
    }

    std::cout << "Logger initialized. Log file: " << logFilename << std::endl;
    std::cout << "Default level: ";
    switch (Logger::getInstance().getDefaultLevel()) {
        case LogLevel::ERROR: std::cout << "ERROR (0)"; break;
        case LogLevel::WARNING: std::cout << "WARNING (1)"; break;
        case LogLevel::INFO: std::cout << "INFO (2)"; break;
    }
    info();

    LogWorker worker;
    worker.start();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit" || line == "exit") {
            break;
        }

        if (line == "help") {
           info();
            continue;
        }

        if (line.substr(0, 9) == "set_level") {
            std::string levelStr = line.substr(10);
            if (!levelStr.empty()) {
                LogLevel newLevel = parseLogLevel(levelStr);
                Logger::getInstance().setDefaultLevel(newLevel);
                std::cout << "Default level changed to: ";
                switch (newLevel) {
                    case LogLevel::ERROR: std::cout << "ERROR"; break;
                    case LogLevel::WARNING: std::cout << "WARNING"; break;
                    case LogLevel::INFO: std::cout << "INFO"; break;
                }
                std::cout << std::endl;
            } else {
                std::cout << "Usage: set_level <level>" << std::endl;
            }
            continue;
        }

        std::istringstream iss(line);
        std::string firstWord;
        if (iss >> firstWord) {
            std::string rest;
            std::getline(iss, rest);

            if (!rest.empty()) {
                std::string lastToken;
                std::istringstream restIss(rest);
                std::string token;
                while (restIss >> token) {
                    lastToken = token;
                }

                LogLevel level = parseLogLevel(lastToken);
                if (lastToken == "ERROR" || lastToken == "error" || lastToken == "0" ||
                    lastToken == "WARNING" || lastToken == "warning" || lastToken == "1" ||
                    lastToken == "INFO" || lastToken == "info" || lastToken == "2") {

                    size_t pos = line.rfind(lastToken);
                    std::string message = line.substr(0, pos);

                    while (!message.empty() && message.back() == ' ') {
                        message.pop_back();
                    }
                    if (!message.empty()) {
                        worker.enqueueLog(message, level);
                    }
                } else {
                    worker.enqueueLog(line, Logger::getInstance().getDefaultLevel());
                }
            } else {
                worker.enqueueLog(firstWord, Logger::getInstance().getDefaultLevel());
            }
        }
    }

    worker.stop();
    Logger::getInstance().flush();

    std::cout << "End!" << std::endl;
    return 0;
}