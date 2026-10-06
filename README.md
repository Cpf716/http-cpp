# HTTP

HTTP-CPP is a proof-of-concept HTTP server written in C++. It implements many of the same features as production libraries, such as thread pooling, connection keep-alive, error handling, and more. While it is not intended for production use, I hope it can help others learn more about the HTTP protocol, parallel computing, and socket programming, just as I learned while developing it.

## API Reference

```
int port = 8080;

http::server server;

server.onrequest([](http::request request) {
    http::header::map headers = {
        { "Content-Type", "text/plain" }
    };

    if (request.method() == "GET" && request.url() == "/api/ping") {
        headers["Connection"] = std::string("keep-alive");
        headers["Keep-Alive"] = std::string("timeout=5");

        return http::response("Hello, world!", headers);
    }

    return http::response(http::NOT_FOUND, "Cannot " + request.method() + " " + request.url(), headers);
});

server.listen(port, []{ std::cout << "Server listening on port " + to_string(port) + "...\n"; });
```