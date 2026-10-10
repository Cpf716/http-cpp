//
//  logger.h
//  http
//
//  Created by Corey Ferguson on 2/5/26.
//

#ifndef logger_h
#define logger_h

#include <iostream>

enum class logger_level { none, some, more, most };
class logger {
    // Member Fields

    logger_level _level = logger_level::some;
public:
    // Constructors

    logger();

    logger(const logger_level level);

    // Member Functions

    void         always(const std::string message);

    void         error(const std::string message);

    logger_level level() const;

    void         log(const logger_level level, const std::string message);

    void         more(const std::string message);

    void         some(const std::string message);
};

#endif /* logger_h */
