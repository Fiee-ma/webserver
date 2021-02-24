#include "../webserver/http/http_server.h"
#include "../webserver/log.h"

static server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void run() {
    server_name::http::HttpServer::ptr server(new server_name::http::HttpServer);
    server_name::Address::ptr addr = server_name::Address::LookupAnyIPAdress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/tmp/preamble-106dac.pch", [](server_name::http::HttpRequest::ptr req
                , server_name::http::HttpResponse::ptr rsp
                , server_name::http::HttpSession::ptr sessiom) {
                rsp->setBody(req->toString());
                return 0;
    });
    sd->addGlobServlet("/tmp/*", [](server_name::http::HttpRequest::ptr req
                , server_name::http::HttpResponse::ptr rsp
                , server_name::http::HttpSession::ptr sessiom) {
                rsp->setBody("Glob:\r\n" + req->toString());
                return 0;
    });

    server->start();
}

int main() {
    server_name::IOManager iom(2);
    iom.schedule(run);

    return 0;
}
