#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cstdio>


// #define LOGGER_ENABLE_SOURCE_INFO 0
// #define LOGGER_ENABLE_TIMESTAMP 0

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    // Log stream class for supporting << operator
    class LogStream {
    public:
        LogStream(Logger& logger, Level level, const char* file, const char* func, int line) 
            : logger_(logger), level_(level), file_(file), func_(func), line_(line) {}
        
        // Disable copy constructor and assignment operator
        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;
        
        // Allow move constructor
        LogStream(LogStream&& other) noexcept
            : logger_(other.logger_), level_(other.level_), file_(other.file_), func_(other.func_), line_(other.line_), message_(std::move(other.message_)) {
            other.message_.str(""); // Clear source object's message
        }
        
        // Disable move assignment operator because Logger reference cannot be reassigned
        LogStream& operator=(LogStream&&) = delete;
        
        ~LogStream() {
            if (!message_.str().empty()) {
                logger_.output(level_, message_.str(), file_, func_, line_);
            }
        }
        
        // Support << operator
        template<typename T>
        LogStream& operator<<(const T& value) {
            message_ << value;
            return *this;
        }
        
        // Support stream operator
        LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            message_ << manip;
            return *this;
        }

    private:
        Logger& logger_;
        Level level_;
        const char* file_;
        const char* func_;
        int line_;
        std::ostringstream message_;
    };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // Set log level
    void setLevel(Level level) {
        current_level_ = level;
    }

    // Stream log output methods with file info
    LogStream debug(const char* file = __FILE__, const char* func = __FUNCTION__, int line = __LINE__) {
        return LogStream(*this, Level::DEBUG, file, func, line);
    }

    LogStream info(const char* file = __FILE__, const char* func = __FUNCTION__, int line = __LINE__) {
        return LogStream(*this, Level::INFO, file, func, line);
    }

    LogStream warning(const char* file = __FILE__, const char* func = __FUNCTION__, int line = __LINE__) {
        return LogStream(*this, Level::WARNING, file, func, line);
    }

    LogStream error(const char* file = __FILE__, const char* func = __FUNCTION__, int line = __LINE__) {
        return LogStream(*this, Level::ERROR, file, func, line);
    }

    // Original template methods (maintain compatibility)
    template<typename... Args>
    void debug(const Args&... args) {
        if (current_level_ <= Level::DEBUG) {
            log(Level::DEBUG, args...);
        }
    }

    template<typename... Args>
    void info(const Args&... args) {
        if (current_level_ <= Level::INFO) {
            log(Level::INFO, args...);
        }
    }

    template<typename... Args>
    void warning(const Args&... args) {
        if (current_level_ <= Level::WARNING) {
            log(Level::WARNING, args...);
        }
    }

    template<typename... Args>
    void error(const Args&... args) {
        if (current_level_ <= Level::ERROR) {
            log(Level::ERROR, args...);
        }
    }

    // Formatted output
    template<typename... Args>
    void log(Level level, const Args&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        output(level, oss.str());
    }

private:
    Logger() : current_level_(Level::INFO) {}
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void output(Level level, const std::string& message) {
        output(level, message, "", "", 0);
    }

    void output(Level level, const std::string& message, const char* file, const char* func, int line) {
        if (level < current_level_) {
            return;
        }
        
        std::ostringstream line_prefix;

#if LOGGER_ENABLE_TIMESTAMP
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        line_prefix << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
                    << "." << std::setfill('0') << std::setw(3) << ms.count() << "]";
#endif

#if LOGGER_ENABLE_SOURCE_INFO
        if (file && file[0] != '\0') {
            if (line_prefix.tellp() > 0) line_prefix << ' ';
            line_prefix << "[" << getFileName(file) << ":" << line << " " << func << "]";
        }
#endif

        if (line_prefix.tellp() > 0) {
            std::cout << line_prefix.str() << ' ';
        }

        std::cout << getLevelString(level) << ": " << message << std::endl;
    }

    std::string getLevelString(Level level) {
        switch (level) {
            case Level::DEBUG:   return "DEBUG";
            case Level::INFO:    return "INFO ";
            case Level::WARNING: return "WARN ";
            case Level::ERROR:   return "ERROR";
            default:             return "UNKNOWN";
        }
    }

    std::string getFileName(const char* filePath) {
        std::string path(filePath);
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos) {
            return path.substr(pos + 1);
        }
        return path;
    }

    Level current_level_;
};

// Convenient macro definitions (maintain original way)
#define LOG_DEBUG(...)   Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...)    Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARNING(...) Logger::getInstance().warning(__VA_ARGS__)
#define LOG_ERROR(...)   Logger::getInstance().error(__VA_ARGS__)

// Stream operator macro definitions with file info
#define LOG_DEBUG_STREAM   Logger::getInstance().debug(__FILE__, __FUNCTION__, __LINE__)
#define LOG_INFO_STREAM    Logger::getInstance().info(__FILE__, __FUNCTION__, __LINE__)
#define LOG_WARNING_STREAM Logger::getInstance().warning(__FILE__, __FUNCTION__, __LINE__)
#define LOG_ERROR_STREAM   Logger::getInstance().error(__FILE__, __FUNCTION__, __LINE__) 