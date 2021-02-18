#include "../webserver/sylar.h"
#include "../webserver/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

server_name::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int sock = 0;
void test_fiber() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    //addr.sin_addr.s_addr = inet_addr("110.242.68.3");
    inet_pton(AF_INET, "110.242.68.3", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr *)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        SYLAR_LOG_INFO(g_logger) << "add event errno " << errno << " " <<
            strerror(errno);
        server_name::IOManager::GetThis()->addEvent(sock, server_name::IOManager::READ, [](){
            SYLAR_LOG_INFO(g_logger) << "read callback";
        });
        server_name::IOManager::GetThis()->addEvent(sock, server_name::IOManager::WRITE, [](){
            SYLAR_LOG_INFO(g_logger) << "write callback";
        });
        server_name::IOManager::GetThis()->cancelEvent(sock, server_name::IOManager::READ);
    } else {
        SYLAR_LOG_INFO(g_logger) << "else" << errno << " " << strerror(errno);
    }
}

void test1() {
    server_name::IOManager iom(2, true, "test");
    iom.schedule(&test_fiber);
}

server_name::Timer::ptr m_timer;
void test_timer() {
    server_name::IOManager io(2, true, "test");
    m_timer = io.addTimer(500, [](){
        SYLAR_LOG_INFO(g_logger) << "hello timer";
        static int i = 0;
        if(++i == 5) {
            //m_timer->reset(2000, true);
            m_timer->cancel();
        }
    }, true);
}

int main() {
//    test1();
    test_timer();
    return 0;
}
