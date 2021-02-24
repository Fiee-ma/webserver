#ifndef __WEBSERVER_HTTP_PARSER_H__
#define __WEBSERVER_HTTP_PARSER_H__

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace server_name {
namespace http {

class HttpRequestParser {
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;

    HttpRequestParser();

    size_t execute(char *data, size_t len);
    int isFinished();
    int hasError();
    void setError(int v) { m_error = v;}

    HttpRequest::ptr getData() const { return m_data;}

    uint64_t getContentLength();
    const http_parser &getParser() const { return m_parser;}

public:
    /// 返回HttpRequest协议解析的缓存大小
    static uint64_t GetHttpRequestBufferSize();
    /// 返回HttpRequest协议最大消息体的大小
    static uint64_t GetHttpRequestMaxBodySize();
private:
    http_parser m_parser;
    HttpRequest::ptr m_data;
    /// 1000: invalid method;
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;
};

class HttpResponseParser {
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;

    HttpResponseParser();

    size_t execute(char *data, size_t len, bool chunk);
    int isFinished();
    int hasError();
    void setError(int v) { m_error = v;}

    HttpResponse::ptr getData() const { return m_data;}
    uint64_t getContentLength();
    /// 返回HttpResponse协议解析的缓存大小
    static uint64_t GetHttpResponseBufferSize();
    /// 返回HttpResponse协议最大消息体的大小
    static uint64_t GetHttpResponseMaxBodySize();

    const httpclient_parser &getParser() const { return m_parser;}
private:
    httpclient_parser m_parser;
    HttpResponse::ptr m_data;
    int m_error;
};


}

}
#endif
