//
//  http.h
//  http
//
//  Created by Corey Ferguson on 1/28/26.
//

#ifndef http_h
#define http_h

#include "logger.h"
#include "url.h"
#include "util.h"
#include "socket.h"
#include <set>
#include <sstream>

namespace http {
    // Non-Member Fields

    static const std::string http_version = "HTTP/1.1";

    static constexpr size_t  keep_alive_max = 200;

    static constexpr size_t  keep_alive_timeout = 5;

    static constexpr size_t  timeout = 30;

    // Typedef

    enum status_code {
        UNKNOWN_ERROR = 0,
        OK = 200,
        NO_CONTENT = 204,
        FOUND = 302,
        TEMPORARY_REDIRECT = 307,
        PERMANENT_REDIRECT = 308,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
    };

    struct error: public std::exception {
        // Constructors

        error(const status_code status);

        error(const status_code status, const std::string text);

        // Member Fields

        status_code status() const;

        std::string status_text() const;

        std::string text() const;

        const char* what() const throw();
    private:
        // Member Fields

        status_code _status;
        std::string _status_text;
        std::string _text;
    };

    struct header {
        // Typedef

        using map = std::map<std::string, header>;

        // Constructors

        header();

        header(const char* value);

        header(const int value);

        header(const std::string value);

        header(std::vector<std::string> value);

        // Operators

        operator                 int();

        operator                 std::string();

        operator                 std::vector<std::string>();

        int                      operator=(const int value);

        std::string              operator=(const std::string value);

        std::vector<std::string> operator=(std::vector<std::string> value);

        bool                     operator==(const char* value);

        bool                     operator==(const int value);

        bool                     operator==(const std::string value);

        bool                     operator==(const header value);

        bool                     operator!=(const char* value);

        bool                     operator!=(const int value);

        bool                     operator!=(const std::string value);

        bool                     operator!=(const header value);

        // Member Functions

        int                      int_value() const;

        std::vector<std::string> list();

        std::string              str() const;
    private:
        // Member Fields

        int                      _int;
        std::vector<std::string> _list;
        std::string              _str;

        // Member Functions

        int                      _set(const int value);

        std::string              _set(const std::string value);

        std::vector<std::string> _set(std::vector<std::string> value);
    };

    struct request {
        // Constructors

        request(const std::string method, const std::string url, header::map headers = {}, const std::string body = "");

        // Member Functions

        std::string     body() const;

        header::map     headers();

        std::string     method() const;

        url::param::map params();

        std::string     url() const;
    private:
        // Member Fields

        std::string     _body;
        header::map     _headers;
        std::string     _method;
        url::param::map _params;
        std::string     _url;
    };

    class response {
        // Member Fields

        std::string _str;
    public:
        // Constructors

        response(const std::string text, header::map headers);

        response(const status_code status, const std::string text, header::map headers, const bool date = true);

        // Member Functions

        std::string str() const;
    };

    class server {
        // Typedef

        class connection_worker {
            std::condition_variable           _cv;
            std::mutex                        _mutex;
            bool                              _stop = false;
            std::queue<std::function<void()>> _tasks;
            std::thread                       _worker;
        public:
            // Constructors

            connection_worker();

            ~connection_worker();

            // Member Functions

            void submit(std::function<void()> task);
        };

        // Member Fields

        std::function<class response(class request)>            _handler;
        logger*                                                 _logger = NULL;
        sockex::tcp_server*                                     _server = NULL;
        std::atomic<bool>                                       _stop = false;
    public:
        // Constructors

        server(class logger* logger = new class logger());

        ~server();

        // Member Functions

        void listen(const int port, std::function<void()> handler = {});

        void onrequest(std::function<class response(class request)> handler);
    };

    // Non-Member Functions

    response    redirect(header::map& headers, const std::string location);

    response    redirect(header::map& headers, const status_code status, const std::string location);

    std::string statusstr(const status_code status);
}

#endif /* http_h */
