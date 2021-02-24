#include "../webserver/tcp_server.h"
#include "../webserver/log.h"
#include "../webserver/iomanager.h"
#include "../webserver/iobytearray.h"
#include <errno.h>
#include <string.h>

static server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

class EchoServer : public server_name::TcpServer {
public:
    EchoServer(int type);
    void handleClient(server_name::Socket::ptr client);
private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    :m_type(type){
}

void EchoServer::handleClient(server_name::Socket::ptr client) {
    WEBSERVER_LOG_INFO(g_logger) << "handleClient " << *client;
    server_name::ByteArray::ptr ba(new server_name::ByteArray);
    while(true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if(rt == 0) {
            WEBSERVER_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if(rt < 0) {
            WEBSERVER_LOG_INFO(g_logger) << "client error rt=: " << rt
                << " error=" << errno << "errstr=" << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        if(m_type == 1) {
            //WEBSERVER_LOG_INFO(g_logger) << ba->toString();
            std::cout << ba->toString();
        } else {
            WEBSERVER_LOG_INFO(g_logger) << ba->toHexString();
        }
    }
}

int type = 1;

void run() {
    EchoServer::ptr es(new EchoServer(type));
    auto addr = server_name::Address::LookupAny("0.0.0.0:8020", AF_INET);
    while(!es->bind(addr)) {
        sleep(2);
    }
    es->start();
}

int main(int argc, char **argv) {
    if(argc < 2) {
        WEBSERVER_LOG_ERROR(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return -1;
    }

    if(strcmp(argv[1], "-t")) {
        type = 2;
    }
    server_name::IOManager iom(2, true, "test");
    iom.schedule(run);

    return 0;
}
