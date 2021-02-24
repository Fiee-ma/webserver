#ifndef __WEBSERVER_HTTP_CONNECTION_H__
#define __WEBSERVER_HTTP_CONNECTION_H__

#include "http_parser.h"
#include "../socket_stream.h"
#include "../socket.h"
#include <memory>

namespace server_name {
namespace http {

class HttpConnection : public SocketStream {
public:
    typedef std::shared_ptr<HttpConnection> ptr;

    /**
     * 构造函数　sock是Socket类型
     * owner表示是否托管
     */
    HttpConnection(Socket::ptr sock, bool owner = true);
    /// 接收http响应
    HttpResponse::ptr recvResponse();
    /**
     * 发送Http请求
     * rsp Http响应
     * -1表示出错
     * 0表示对方关闭
     * 1表示发送成功
     */
    int sendRequest(HttpRequest::ptr req);
};

}
}



#endif
