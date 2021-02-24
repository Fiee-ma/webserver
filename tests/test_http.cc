#include "../webserver/http/http.h"
#include "../webserver/log.h"

static server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void test_request() {
    server_name::http::HttpRequest::ptr req(new server_name::http::HttpRequest);
    req->setHeader("host", "www.baidu.com");
    req->setBody("Hello baidu");

    req->dump(std::cout) << std::endl;
}

void test_response() {
    server_name::http::HttpResponse::ptr rsp(new server_name::http::HttpResponse);
    rsp->setHeader("X-X", "marulong");
    rsp->setBody("Hello marulong");
    rsp->setStatus((server_name::http::HttpStatus)400);

    rsp->dump(std::cout) << std::endl;
}

int main() {
    //test_http();
    test_response();

    return 0;
}
