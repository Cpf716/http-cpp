//
//  logger.cpp
//  http
//
//  Created by Corey Ferguson on 2/5/26.
//

#include "logger.h"

// Constructors

logger::logger() : logger(logger_level::some) { }

logger::logger(const logger_level level) {
    this->_level = level;
}

// Member Functions

void logger::always(const std::string message) {
    std::cout << message << std::endl;
}

void logger::error(const std::string message) {
    this->always("Error: " + message);
}

enum logger_level logger::level() const {
    return this->_level;
}

void logger::log(const logger_level level, const std::string message) {
    if (this->level() >= level) this->always(message);
}

void logger::more(const std::string message) {
    this->log(logger_level::more, message);
}

void logger::some(const std::string message) {
    this->log(logger_level::some, message);
}
