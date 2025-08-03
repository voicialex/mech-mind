#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    // 日志流类，用于支持 << 操作符
    class LogStream {
    public:
        LogStream(Logger& logger, Level level) 
            : logger_(logger), level_(level) {}
        
        // 禁用拷贝构造函数和赋值操作符
        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;
        
        // 允许移动构造函数
        LogStream(LogStream&& other) noexcept
            : logger_(other.logger_), level_(other.level_), message_(std::move(other.message_)) {
            other.message_.str(""); // 清空源对象的消息
        }
        
        // 禁用移动赋值操作符，因为Logger引用不能重新赋值
        LogStream& operator=(LogStream&&) = delete;
        
        ~LogStream() {
            if (!message_.str().empty()) {
                logger_.output(level_, message_.str());
            }
        }
        
        // 支持 << 操作符
        template<typename T>
        LogStream& operator<<(const T& value) {
            message_ << value;
            return *this;
        }
        
        // 支持流操作符
        LogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            message_ << manip;
            return *this;
        }

    private:
        Logger& logger_;
        Level level_;
        std::ostringstream message_;
    };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // 设置日志级别
    void setLevel(Level level) {
        current_level_ = level;
    }

    // 流式日志输出方法
    LogStream debug() {
        return LogStream(*this, Level::DEBUG);
    }

    LogStream info() {
        return LogStream(*this, Level::INFO);
    }

    LogStream warning() {
        return LogStream(*this, Level::WARNING);
    }

    LogStream error() {
        return LogStream(*this, Level::ERROR);
    }

    // 原有的模板方法（保持兼容性）
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

    // 格式化输出
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
        if (level < current_level_) {
            return;
        }
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
                  << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
                  << getLevelString(level) << ": " << message << std::endl;
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

    Level current_level_;
};

// 便捷宏定义（保持原有方式）
#define LOG_DEBUG(...)   Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...)    Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARNING(...) Logger::getInstance().warning(__VA_ARGS__)
#define LOG_ERROR(...)   Logger::getInstance().error(__VA_ARGS__)

// 流式操作符宏定义
#define LOG_DEBUG_STREAM   Logger::getInstance().debug()
#define LOG_INFO_STREAM    Logger::getInstance().info()
#define LOG_WARNING_STREAM Logger::getInstance().warning()
#define LOG_ERROR_STREAM   Logger::getInstance().error() 