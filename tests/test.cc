#include"../webserver/log.h"
#include"../webserver/util.h"

int main(int argc, char **argv){
    server_name::Logger::ptr logger(new server_name::Logger);
    std::cout<<"construct logger succces"<<std::endl;
    logger->addAppender(server_name::LogAppender::ptr(new server_name::StdoutLogAppender));

//    server_name::LogEvent::ptr event(new server_name::LogEvent(__FILE__, __LINE__, 0, server_name::GetThreadId(),
//                server_name::GetFiberId(), time(0)));

//    logger->log(server_name::LogLevel::Level::DEBUG, event);
//    std::cout<<"hello marulong log"<<std::endl;

    server_name::FileLogAppender::ptr file_appender(new server_name::FileLogAppender("/home/marulong/sylar/tests/log.txt"));
    server_name::LogFormatter::ptr fmt(new server_name::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(server_name::LogLevel::ERROR);

    logger->addAppender(file_appender);
    SYLAR_LOG_INFO(logger)<< "test INFO";
    SYLAR_LOG_WARN(logger)<< "test WARN";

    SYLAR_LOG_FMT_ERROR(logger, "test macro fmt %s", "aa");
    SYLAR_LOG_FMT_INFO(logger, "haha hahah %d", 250);
    auto l = server_name::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_DEBUG(l)<< "xxxx";

    return 0;
}
