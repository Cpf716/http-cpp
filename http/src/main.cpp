//
//  main.cpp
//  http
//
//  Created by Corey Ferguson on 1/28/26.
//

#include "http.h"
#include "url.h"

using namespace std;

static constexpr int port = 8080;

int main(int argc, const char* argv[]) {
    class logger logger;

    http::server server(&logger);

    server.onrequest([&logger](http::request request) {
        logger.some("url: " + request.url() + ", body: " + (request.body().empty() ? "null" : request.body()));

        http::header::map headers = {
            { "Content-Type", "text/plain" }
        };

        if (request.method() == "GET" && request.url() == "/api/ping") {
            headers["Connection"] = string("keep-alive");
            headers["Keep-Alive"] = string("timeout=" + to_string(http::keep_alive_timeout) + ";max=" + to_string(http::keep_alive_max));

            return http::response("Hello, world!", headers);
        }

        return http::response(http::NOT_FOUND, "Cannot " + request.method() + " " + request.url(), headers);
    });

    server.listen(port, []{ cout << "Server listening on port " + to_string(port) + "...\n"; });
}
