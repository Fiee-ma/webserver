#include"../sylar/log.h"
#include"../sylar/util.h"

int main(int argc, char **argv){
    sylar::Logger::ptr logger(new sylar::Logger);
    std::cout<<"construct logger succces"<<std::endl;
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

//    sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, sylar::GetThreadId(),
//                sylar::GetFiberId(), time(0)));

//    logger->log(sylar::LogLevel::Level::DEBUG, event);
//    std::cout<<"hello marulong log"<<std::endl;

    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("/home/marulong/sylar/tests/log.txt"));
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel::ERROR);

    logger->addAppender(file_appender);
    SYLAR_LOG_INFO(logger)<< "test INFO";
    SYLAR_LOG_WARN(logger)<< "test WARN";

    SYLAR_LOG_FMT_ERROR(logger, "test macro fmt %s", "aa");
    SYLAR_LOG_FMT_INFO(logger, "haha hahah %d", 250);
    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_DEBUG(l)<< "xxxx";

    return 0;
}
