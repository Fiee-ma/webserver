#ifndef __WEBSERVER_HTTP_SESSION_H__
#define __WEBSERVER_HTTP_SESSION_H__

#include "http_parser.h"
#include "../socket_stream.h"
#include "../socket.h"
#include <memory>

namespace server_name {
namespace http {

class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;

    /**
     * 构造函数　sock是Socket类型
     * owner表示是否托管
     */
    HttpSession(Socket::ptr sock, bool owner = true);
    /// 接收http请求
    HttpRequest::ptr recvRequest();
    /**
     * 发送Http响应
     * rsp Http响应
     * -1表示出错
     * 0表示对方关闭
     * 1表示发送成功
     */
    int sendResponse(HttpResponse::ptr rsp);
};

}
}



#endif
