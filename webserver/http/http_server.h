#ifndef __WEBSERVER_HTTP_SERVER_H__
#define __WEBSERVER_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace server_name {
namespace http {

class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer(bool keeplive = false
            , IOManager *worker = IOManager::GetThis()
            , IOManager *accept_worker = IOManager::GetThis());
    /**
     * @brief 获取ServletDispatch
     */
    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}

    /**
     * @brief 设置ServletDispatch
     */
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}

    virtual void setName(const std::string& v) override;

protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;

};

}

}


#endif
