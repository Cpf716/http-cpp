//
//  http.cpp
//  http
//
//  Created by Corey Ferguson on 1/28/26.
//

#include "http.h"

namespace http {
    // Non-Member Functions

    std::string statusstr(const status_code status) {
        switch (status) {
            case UNKNOWN_ERROR:
                return "Unknown error";
            case OK:
                return "OK";
            case NO_CONTENT:
                return "No Content";
            case FOUND:
                return "Found";
            case TEMPORARY_REDIRECT:
                return "Temporary Redirect";
            case PERMANENT_REDIRECT:
                return "Permanent Redirect";
            case BAD_REQUEST:
                return "Bad Request";
            case UNAUTHORIZED:
                return "Unauthorized";
            case NOT_FOUND:
                return "Not Found";
            case INTERNAL_SERVER_ERROR:
                return "Internal Server Error";
            default:
                break;
        }
        
        return "";
    }

    request parse_request(const std::string message) {
        std::istringstream iss(message);
        std::string        line;

        getline(iss, line);

        std::vector<std::string> tokens = ::tokens(line);

        if (!(tokens.size() == 3 && tokens[2] == http_version))
            throw http::error(BAD_REQUEST);

        std::string method = toupperstr(tokens[0]),
                     target = tokens[1];
        header::map headers;

        while (getline(iss, line)) {
            std::vector<std::string> header = split(line, ":");

            if (header.size() == 1)
                break;

            headers[tolowerstr(header[0])] = trim(line.substr(header[0].length() + 1));
        }

        auto        it = headers.find("content-length");
        std::string body = "";

        if (it != headers.end()) {
            std::vector<std::string> value;

            while (getline(iss, line))
                value.push_back(trim_end(line));

            body = join(value, "\r\n");
            body = body.substr(0, std::min((int) body.length(), (int) (* it).second));
        }

        return request(method, target, headers, body);
    }

    response redirect(header::map& headers, const status_code status, const std::string location) {
        headers["Location"] = location;

        return response(status, statusstr(status) + ". Redirecting to " + location, headers);
    }

    response redirect(header::map& headers, const std::string location) {
        return redirect(headers, FOUND, location);
    }

    response::response(const std::string text, header::map headers) : response(OK, text, headers) { }

    response::response(const status_code status, const std::string text, header::map headers, const bool date) {
        this->_str += http_version;
        this->_str += " ";
        this->_str += std::to_string(status);
        this->_str += " ";
        this->_str += statusstr(status);
        this->_str += "\r\n";

        // Response headers
        if (date) {
            time_t now = time(0);
            tm*    gmtm = gmtime(&now);
            char*  dt = asctime(gmtm);
            
            std::vector<std::string> tokens = ::tokens(std::string(dt));

            this->_str += "Date: ";

            std::string day = tokens[0],
                         month = tokens[1],
                         date = tokens[2],
                         time = tokens[3],
                         year = tokens[4];

            this->_str += day;
            this->_str += ", ";
            this->_str += date;
            this->_str += " ";
            this->_str += month;
            this->_str += " ";
            this->_str += year;
            this->_str += " ";
            this->_str += time;
            this->_str += " GMT";
        }

        for (const auto& [key, value]: headers) {
            this->_str += "\r\n";
            this->_str += key;
            this->_str += ": ";
            this->_str += value.str();
        }
        
        if (text.length()) {
            if (headers.find("Transfer-Encoding") == headers.end()) {
                this->_str += "\r\n";
                this->_str += "Content-Length: ";
                this->_str += std::to_string(text.length());
            }

            this->_str += "\r\n\r\n";
            this->_str += text;
        }
    }

    // Constructors

    error::error(const status_code status) {
        this->_status = status;
        this->_status_text = statusstr(this->status());
        this->_text = "";
    }

    error::error(const status_code status, const std::string text) {
        this->_status = status;
        this->_status_text = statusstr(this->status());
        this->_text = text;
    }

    header::header() {
        this->_set("");
    }

    header::header(const char* value) {
        this->_set(std::string(value));
    }

    header::header(const int value) {
        this->_set(value);
    }

    header::header(const std::string value) {
        this->_set(value);
    }

    header::header(std::set<std::string> value) {
        this->_set(value);
    }

    request::request(const std::string method, const std::string url, header::map headers, const std::string body) {
        this->_method = method;

        class url url_obj(url);

        this->_url = url_obj.target();
        this->_params = url_obj.params();
        this->_headers = headers;
        this->_body = body;
    }

    server::server(class logger* logger) {
        this->_logger = logger;
    }

    server::~server() {
        this->_stop.store(true);
        this->_server->close();
    }

    // Operators

    header::operator int() {
        return this->int_value();
    }

    header::operator std::string() {
        return this->str();
    }

    header::operator std::set<std::string>() {
        return this->list();
    }

    int header::operator=(const int value) {
        return this->_set(value);
    }

    std::string header::operator=(const std::string value) {
        return this->_set(value);
    }

    std::set<std::string> header::operator=(std::set<std::string> value) {
        return this->_set(value);
    }

    bool header::operator==(const char* value) {
        return this->str() == std::string(value);
    }

    bool header::operator==(const header value) {
        return this->str() == value.str();
    }

    bool header::operator==(const int value) {
        return this->int_value() == value;
    }

    bool header::operator==(const std::string value) {
        return this->str() == value;
    }

    bool header::operator!=(const char* value) {
        return !(*this == value);
    }

    bool header::operator!=(const header value) {
        return !(*this == value);
    }

    bool header::operator!=(const int value) {
        return !(*this == value);
    }

    bool header::operator!=(const std::string value) {
        return !(*this == value);
    }

    // Member Functions

    int header::_set(const int value) {
        this->_int = value;
        this->_str = std::to_string(this->int_value());
        this->_list = { this->str() };

        return this->int_value();
    }

    std::string header::_set(const std::string value) {
        this->_str = value;
        this->_int = parse_int(this->str());
        
        this->_list.clear();
        
        for (std::string item: split(value, ","))
            this->_list.insert(trim(item));

        return this->str();
    }

    std::set<std::string> header::_set(std::set<std::string> value) {
        this->_list = value;
        
        std::vector<std::string> list;
        
        for (std::string item: this->list())
            list.push_back(item);
        
        this->_str = join(list, ",");
        this->_int = INT_MIN;

        return this->list();
    }

    std::string request::body() const {
        return this->_body;
    }

    header::map request::headers() {
        return this->_headers;
    }

    int header::int_value() const {
        return this->_int;
    }

    std::set<std::string> header::list() {
        return this->_list;
    }

    void server::listen(const int port, std::function<void()> handler) {
        this->_server = new sockex::tcp_server(port, [this](sockex::tcp_server::connection* connection) {
            // Number of requests received
            auto requestc = std::make_shared<std::atomic<int>>(0);
        
            // Listen for connection timeout
            std::thread([connection](std::shared_ptr<std::atomic<int>> requestc) {
                for (size_t i = 0; i < http::timeout * 10 && !requestc->load(); i++)
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 10));

                // Cancel timeout disconnect, if request was received
                if (requestc->load()) return;

                // Silently catch EBADF
                requestc->store(1);
                connection->close();
            }, requestc).detach();

            // Wait for non-empty request
            for (;;) {
                try {
                    std::string request = connection->recv();

                    if (request.empty()) continue;

                    this->_logger->more(request + "\r\n");

                    // Cancel initial timeout / increment request ID
                    requestc->fetch_add(1);

                    auto handle_response = [this, connection](class response response) {
                        this->_logger->more(response.str() + "\r\n");

                        connection->send(response.str());
                    };

                    try {
                        class request request_obj = parse_request(request);

                        if (request_obj.headers()["host"].str().length()) {
                            class response response = this->_handler(request_obj);

                            handle_response(response);

                            size_t request_id = requestc->load();

                            // Disconnect, if keep_alive_max was reached
                            if (request_id >= keep_alive_max)
                                return connection->close();

                            // Keep-alive
                            std::thread([request_id, connection](std::shared_ptr<std::atomic<int>> requestc) {
                                for (int i = 0; i < keep_alive_timeout * 10 && request_id == requestc->load(); i++)
                                    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 10));

                                // Disconnect, if keep-alive expired before receiving another request
                                if (request_id == requestc->load())
                                    connection->close();
                            }, requestc).detach();

                            // Disconnect immediately, if request lacks 'host' header
                        } else {
                            handle_response(response(BAD_REQUEST, std::to_string(0), {
                                { "Connection", "close" },
                                { "Transfer-Encoding", "chunked "}
                            }));

                            return connection->close();
                        }

                        // Disconnect immediately, if malformed HTTP request
                    } catch (http::error& e) {
                        handle_response(response(BAD_REQUEST, e.text(), {
                            { "Connection", "close" }
                        }, false));
                
                        return connection->close();
                    }
                } catch (sockex::error& e) {
                    // Suppress EBADF, if connection timed out
                    if (requestc->load()) return;
                        
                    throw e;
                }
            }
        });

        handler();

        while (!this->_stop.load())
            continue;
    }

    std::string request::method() const {
        return this->_method;
    }

    void server::onrequest(std::function<class response(class request)> handler) {
        this->_handler = handler;
    }

    url::param::map request::params() {
        return this->_params;
    }

    status_code error::status() const {
        return this->_status;
    }

    std::string error::status_text() const {
        return this->_status_text;
    }

    std::string header::str() const {
        return this->_str;
    }

    std::string response::str() const { return this->_str; }

    std::string error::text() const {
        return this->_text;
    }

    std::string request::url() const {
        return this->_url;
    }

    const char* error::what() const throw() {
        return this->_text.c_str();
    }
}
