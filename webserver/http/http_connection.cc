#include "http_connection.h"
#include "http_parser.h"
#include "../log.h"

namespace server_name {
namespace http {

static Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner){
}

HttpResponse::ptr HttpConnection::recvResponse() {
    HttpResponseParser::ptr parser(new HttpResponseParser);
    uint64_t buffer_size = HttpRequestParser::GetHttpRequestBufferSize();
    std::shared_ptr<char> buffer(
            new char[buffer_size + 1], [](char *ptr) {
                delete []ptr;
            });
    char *data = buffer.get();
    int offset = 0;  //剩余数据长度
    while(true) {
        int len = read(data + offset, buffer_size - offset);
        WEBSERVER_LOG_DEBUG(g_logger) << "len = " << len;
        if(len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        data[len] = '\0';
        size_t nparser = parser->execute(data, len, false);
        if(parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparser;
        if(offset == (int)buffer_size) {  //数据一个也没解析
            close();
            return nullptr;
        }
        if(parser->isFinished()) {
            break;
        }
    }
    auto &client_parser = parser->getParser();
    std::string body;
    if(client_parser.chunked) {
        int len = offset;
        do {
            bool begin = true;
            do {
                if(!begin || len == 0) {
                    int rt = read(data + len, buffer_size - len);
                    if(rt <= 0) {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';
                size_t nparse = parser->execute(data, len, true);
                if(parser->hasError()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if(len == (int)buffer_size) {
                    close();
                    return nullptr;
                }
                begin = false;
            } while(!parser->isFinished());
            //len -= 2;

            WEBSERVER_LOG_DEBUG(g_logger) << "content_len=" << client_parser.content_len;
            if(client_parser.content_len + 2 <= len) {
                body.append(data, client_parser.content_len);
                memmove(data, data + client_parser.content_len + 2
                        , len - client_parser.content_len - 2);
                len -= client_parser.content_len + 2;
            } else {
                body.append(data, len);
                int left = client_parser.content_len - len + 2;
                while(left > 0) {
                    int rt = read(data, left > (int)buffer_size ? (int)buffer_size : left);
                    if(rt <= 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2);
                len = 0;
            }
        } while(!client_parser.chunks_done);
    } else {
        int64_t length = parser->getContentLength();
        if(length > 0) {
            std::string body;
            body.resize(length);

            int len = 0;
            if(length >= offset) {
                memcpy(&body[0], data, offset);
                len = offset;
            } else {
                memcpy(&body[0], data, length);
                len = length;
            }
            length -= offset;
            if(length > 0) {
                if(readFixSize(&body[len], length) <= 0) {
                    close();
                    return nullptr;
                }
            }
            parser->getData()->setBody(body);
        }
    }

//    parser->getData()->init();
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr req) {
    std::stringstream ss;
    ss << *req;
    std::string str = ss.str();
    return writeFixSize(str.c_str(), str.size());
}

}

}
