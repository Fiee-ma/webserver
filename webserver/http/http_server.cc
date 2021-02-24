#include "http_server.h"
#include "../log.h"
#include "../iomanager.h"

namespace server_name {
namespace http {

Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

HttpServer::HttpServer(bool keeplive
    , IOManager *worker, IOManager *accept_worker)
    :TcpServer(worker, accept_worker)
    ,m_isKeepalive(keeplive) {
    m_dispatch.reset(new ServletDispatch);
}

void HttpServer::setName(const std::string& v) {
    TcpServer::setName(v);
    m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client) {
    HttpSession::ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        WEBSERVER_LOG_DEBUG(g_logger) << "request:\n" << *req;
        if(!req) {
            WEBSERVER_LOG_WARN(g_logger) << "recv http request fail, errno="
                << errno << " errstr=" << strerror(errno)
                << " client:" << *client;
            break;
        }

        HttpResponse::ptr rsp( new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
//        rsp->setBody("hello marulong");
        m_dispatch->handle(req, rsp, session);

        session->sendResponse(rsp);
    } while(m_isKeepalive);
    session->close();
}



}
}
