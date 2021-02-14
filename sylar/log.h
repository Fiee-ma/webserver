#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include<iostream>
#include<string>
#include<stdint.h>
#include<memory>
#include<list>
#include<sstream>
#include<fstream>
#include<vector>
#include<map>
#include<functional>
#include<stdarg.h>
#include "util.h"
#include "singleton.h"
#include<algorithm>
#include "thread.h"

#define SYLAR_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, sylar::GetThreadId(), \
                    sylar::GetFiberId(), time(0),sylar::Thread::GetName()))).getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, sylar::GetThreadId(), \
                        sylar::GetFiberId(), time(0), \
                        sylar::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)
//使用保留名 __VA_ARGS__ 把参数传递给宏

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar {

class Logger;
class LoggerManager;
//日志级别
class LogLevel{
public:
    enum Level{
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    /**
     * 将日志级别转换成文本输出
     */
    static const char *ToString(LogLevel::Level level);
    /**
     * 将文本转换成日志级别
     */
    static LogLevel::Level FromString(const std::string &str);
};

//日志事件
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent();
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse
            , uint32_t thread_id, uint32_t fiber_id, uint64_t time, const std::string &thread_name);
    ~LogEvent(){};

    const char *getFile() const { return m_file;}
    int32_t getLine() const {return m_line;}
    uint32_t getElapse() const {return m_elapse;}
    uint32_t getThreadId() const {return m_threadId;}
    uint32_t getFiberId() const { return m_fiberId;}
    uint64_t getTime() const {return m_time;}
    const std::string &getThreadName() const {return m_threadName;}
    std::string getContent() const {return m_ss.str();}
    std::shared_ptr<Logger> getLogger() const {return m_logger;}
    LogLevel::Level getLevel() const {return m_level;}

    std::stringstream &getSS() {return m_ss;}

    void format(const char *fmt, ...);
    void format(const char *fmt, va_list al);
private:
    const char *m_file = nullptr;  //文件名
    int32_t m_line = 0;            //行号
    uint32_t m_elapse = 0;         //程序启动开始到现在的毫秒数
    uint32_t m_threadId = 0;       //线程id
    uint32_t m_fiberId = 0;        //协程id
    uint64_t m_time = 0;           //时间戳
    std::string m_threadName;      //线程名称
    std::stringstream m_ss;         //文件内容

    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    std::stringstream &getSS();
    LogEvent::ptr getEvent() const { return m_event;}

private:
    LogEvent::ptr m_event;
};


//日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string &pattern);

    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    void init();

    std::string getPattern() const { return m_pattern;}
public:
    class FormatItem{
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem(){}
        virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    bool isError() const { return m_error;}
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;
};

//日志输出地
class LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef SpinLock MutexType;
    virtual ~LogAppender(){}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;

    void setFormatter(LogFormatter::ptr val); //设置formatter的值
    LogFormatter::ptr getFormatter(); //获取formatter的值
    void setLevel(LogLevel::Level level) {m_level = level;}
    LogLevel::Level getLevel() const { return m_level;}

protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
    bool m_hasFormatter = false;
    MutexType m_mutex;
};

//日志器
class Logger : public std::enable_shared_from_this<Logger> {
    friend LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef SpinLock MutexType;

    Logger(const std::string &name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);   //输出debug级别的日志
    void info(LogEvent::ptr event);    //输出info级别的日志
    void warn(LogEvent::ptr event);    //输出warn级别的日志
    void error(LogEvent::ptr event);   //输出error级别的日志
    void fatal(LogEvent::ptr event);   //输出fatal级别的日志

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string &str);
    LogFormatter::ptr getFormatter();
    void addAppender(LogAppender::ptr appender);  //添加appender
    void delAppender(LogAppender::ptr appender);  //删除appender
    LogLevel::Level getLevel() const {return m_level;}  //获取日志级别
    void setLevel(LogLevel::Level val) {m_level = val;}  //设置日志级别
    void clearAppender();

    const std::string &getName() const { return m_name;}

    std::string toYamlString();
private:
    std::string m_name;                         //日志名称
    LogLevel::Level m_level;                    //日志级别
    std::list<LogAppender::ptr> m_appenders;    //Appender集合
    LogFormatter::ptr m_formatter;
    Logger::ptr m_root;
    MutexType m_mutex;
};

//输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;

private:
};

//定义输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;

    //重新打开文件，打开成功返回true
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
};

class LoggerManager {
public:
    typedef SpinLock MutexType;
    LoggerManager();
    Logger::ptr getLogger(const std::string &name);

    void init();
    Logger::ptr getRoot() const { return m_root;}

    std::string toYamlString();

private:
    std::map<std::string, Logger::ptr> m_logger;
    Logger::ptr m_root;
    MutexType m_mutex;
};

typedef sylar::Singleton<LoggerManager> LoggerMgr;




}



#endif
















