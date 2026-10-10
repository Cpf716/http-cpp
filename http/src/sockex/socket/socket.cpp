//
//  socket.cpp
//  http
//
//  Created by Corey Ferguson on 1/28/26.
//

#include "socket.h"

namespace sockex {
    // Non-Member Functions

    std::string recv(const int file_descriptor) {
        char buff[buffer_length] = {0};
            
        ssize_t len = ::recv(file_descriptor, buff, buffer_length, 0);

        if (len == -1) throw sockex::error(errno);

        buff[len] = '\0';
        
        return std::string(buff);
    }

    int send(const int file_descriptor, const std::string message) {
        ssize_t len = ::send(file_descriptor, message.c_str(), message.length(), MSG_NOSIGNAL);
            
        if (len == -1) throw sockex::error(errno);
        
        return (int) len;
    }

    // Constructors

    tcp_server::connection::connection(tcp_server* parent, const int file_descriptor) {
        this->_parent = parent;
        this->_file_descriptor = file_descriptor;
    }

    error::error(const int errnum) {
        this->_errnum = errnum;
        this->_what = std::strerror(this->_errnum);
    }

    error::error(const std::string what) {
        this->_what = what;
    }

    tcp_client::tcp_client(const std::string host, const int port) {
        this->_file_descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
            
        if (this->_file_descriptor == -1)
            throw sockex::error(errno);
        
        struct sockaddr_in addr;

        // IPv4
        addr.sin_family = AF_INET;

        // Convert port to network byte order
        addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) == -1)
            throw sockex::error(errno);
        
        // Returns 0 for success, otherwise -1
        if (connect(this->_file_descriptor, (struct sockaddr *)&addr, sizeof(addr))) {
            ::close(this->_file_descriptor);
            
            throw sockex::error(errno);
        }
    }

    tcp_server::tcp_server(const int port, const int backlog) {
        this->_file_descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
            
        if (this->_file_descriptor == -1)
            throw sockex::error(errno);
                
        int opt = 1;
        
        if (setsockopt(this->_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            ::close(this->_file_descriptor);
            
            throw sockex::error(errno);
        }
            
        // Listen for any IP address
        this->_address.sin_addr.s_addr = INADDR_ANY;

        // IPv4
        this->_address.sin_family = AF_INET;

        // Convert port to network byte order
        this->_address.sin_port = htons(port);
        this->_address_length = sizeof(this->_address);
        
        if (bind(this->_file_descriptor, (struct sockaddr *)&this->_address, this->_address_length)) {
            ::close(this->_file_descriptor);
            
            throw sockex::error(errno);
        }
        
        if (listen(this->_file_descriptor, backlog)) {
            ::close(this->_file_descriptor);
            
            throw sockex::error(errno);
        }
        
        this->_listener = std::thread([this]{
            for (;;) {
                if (this->_stop.load()) return;
                
                // Returns nonnegative file descriptor or -1 for error
                int file_descriptor = accept(
                    this->_file_descriptor,
                    (struct sockaddr *)&this->_address, (socklen_t *)&this->_address_length
                );
                
                if (file_descriptor == -1) continue;

                class connection* connection = new class connection(this, file_descriptor);
                {
                    std::lock_guard lock(this->_mutex);

                    this->_connections.push_back(connection);
                }

                this->_pool.submit([this, connection]{
                    this->_handler(connection);
                });
            }
        });
    }

    tcp_server::tcp_server(const int port, const std::function<void(connection*)> handler, const int backlog): tcp_server(port, backlog) {
        this->_handler = handler;
    }

    tcp_server::thread_pool::thread_pool(const size_t size) {
        this->_dispatcher = [this]{
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock lock(this->_mutex);

                    this->_cv.wait(lock, [this]{ return this->_stop || !this->_tasks.empty(); });

                    if (this->_stop) return;
                    
                    task = std::move(this->_tasks.front());

                    this->_tasks.pop();
                }

                counter_guard counter(this->_counter);

                task();
            }
        };

        this->_workers.reserve(size);

        for (int i = 0; i < size; i++)
            this->_workers.emplace_back(this->_dispatcher);
    }

    tcp_server::thread_pool::counter_guard::counter_guard(std::atomic<int>& counter) : _counter(counter) {
        this->_counter.fetch_add(1);
    }

    udp_client::udp_client(const std::string host, const int port) {
        this->_file_descriptor = ::socket(AF_INET, SOCK_DGRAM, 0);
            
        if (this->_file_descriptor == -1)
            throw sockex::error(errno);
        
        this->_address = new struct sockaddr_in();
        
        memset(this->_address, 0, sizeof(* this->_address));
        
        // IPv4
        this->_address->sin_family = AF_INET;

        // Convert port to network byte order
        this->_address->sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &this->_address->sin_addr) == -1) {
            ::close(this->_file_descriptor);
            
            throw sockex::error(errno);
        }
    }

    udp_server::udp_server(const int port) {
        this->_file_descriptor = ::socket(AF_INET, SOCK_DGRAM, 0);
            
        if (this->_file_descriptor == -1)
            throw sockex::error(errno);
        
        // Server address
        struct sockaddr_in addr;
        
        memset(&addr, 0, sizeof(addr));
        
        // Listen for any IP address
        addr.sin_addr.s_addr = INADDR_ANY;

        // IPv4
        addr.sin_family = AF_INET;

        // Convert port to network byte order
        addr.sin_port = htons(port);
        
        if (bind(this->_file_descriptor, (const struct sockaddr *)&addr, sizeof(addr))) {
            ::close(this->_file_descriptor);
            
            throw sockex::error(errno);
        }
        
        // Client address
        this->_address = new struct sockaddr_in();
        
        memset(this->_address, 0, sizeof(* this->_address));
    }

    tcp_server::connection::~connection() { }

    tcp_client::~tcp_client() { }

    tcp_server::~tcp_server() { }

    tcp_server::thread_pool::~thread_pool() {
        {
            std::lock_guard lock(this->_mutex);

            this->_stop = true;
        }

        this->_cv.notify_all();

        for (std::thread& thread: this->_workers)
            thread.join();
    }

    tcp_server::thread_pool::counter_guard::~counter_guard() {
        this->_counter.fetch_sub(1);
    }

    udp_client::~udp_client() { }

    udp_server::~udp_server() { }

    // Member Functions

    void tcp_server::connection::_close() {
        if (::close(this->_file_descriptor))
            throw sockex::error(errno);

        delete this;
    }

    void tcp_server::connection::close() {
        this->_parent->close(this);
    }

    void tcp_client::close() {
        if (::close(this->_file_descriptor)) 
            throw sockex::error(errno);

        delete this;
    }

    void tcp_server::close() {
        this->_stop.store(true);
        
        std::lock_guard lock(this->_mutex);
        
        if (::close(this->_file_descriptor))
            throw sockex::error(errno);
        
        this->_listener.join();

        for (size_t i = 0; i < this->_connections.size(); i++) {
            if (::close(this->_connections[i]->_file_descriptor))
                throw sockex::error(errno);
                
            delete this->_connections[i];
        }
        
        delete this;
    }

    void udp_socket::close() {
        if (::close(this->_file_descriptor))
            throw sockex::error(errno);
        
        delete this->_address;
        delete this;
    }

    bool tcp_server::close(class connection* connection) {
        std::lock_guard lock(this->_mutex);

        auto it = std::find(this->_connections.begin(), this->_connections.end(), connection);

        if (it == this->_connections.end()) return false;

        this->_connections.erase(it);

        connection->_close();

        return true;
    }

    std::vector<tcp_server::connection*> tcp_server::connections() {
        std::lock_guard lock(this->_mutex);

        return this->_connections;
    }

    int error::errnum() const {
        return this->_errnum;
    }

    std::string tcp_server::connection::recv() const {
        return sockex::recv(this->_file_descriptor);
    }

    std::string tcp_client::recv() const {
        return sockex::recv(this->_file_descriptor);
    }

    std::string udp_socket::recvfrom() const {
        char      buff[1024];
        socklen_t addrlen = sizeof(* this->_address);
        ssize_t   len = ::recvfrom(this->_file_descriptor, (char *)buff, 1024, MSG_WAITALL, (struct sockaddr *)this->_address, &addrlen);
        
        if (len == -1) throw sockex::error(errno);
        
        buff[len] = '\0';
        
        return std::string(buff);
    }

    int tcp_server::connection::send(const std::string message) const {
        return sockex::send(this->_file_descriptor, message);
    }

    int tcp_client::send(const std::string message) const {
        return sockex::send(this->_file_descriptor, message);
    }

    int udp_socket::sendto(const std::string message) const {
        ssize_t len = ::sendto(this->_file_descriptor, (const char *)message.c_str(), message.length(), 0, (const struct sockaddr *)this->_address, sizeof(* this->_address));
        
        if (len == -1) throw sockex::error(errno);
        
        return (int) len;
    }

    void tcp_server::thread_pool::submit(std::function<void()> task) {
        {
            std::lock_guard lock(this->_mutex);

            // Increase pool size, if required and below capacity
            // Otherwise enqueue task
            if (this->_counter.load() == this->_workers.size() && this->_counter.load() != thread_pool_max_size)
                this->_workers.emplace_back(this->_dispatcher);

            this->_tasks.push(std::move(task));
        }

        this->_cv.notify_one();
    }

    const char* error::what() const throw() {
        return this->_what.c_str();
    }
}
