# HTTP

HTTP-CPP is a proof-of-concept HTTP server written in C++. It features a configurable thread pool with a default of 8 connections up to maximum of 100 connections, lightning-fast built-in JSON serializer, and more. The server boasts familiar, modern syntax with the performance and portability of C++.

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

server.listen(port, [port]{ std::cout << "Server listening on port " + to_string(port) + "...\n"; });
```