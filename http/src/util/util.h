//
//  util.h
//  http
//
//  Created by Corey Ferguson on 1/28/26.
//

#ifndef util_h
#define util_h

#include <iostream>
#include <random>
#include <sstream>

// Non-Member Functions

bool                     is_int(const std::string value);

bool                     is_number(const std::string value);

std::string              join(std::vector<std::string> values, std::string delimeter);

int                      parse_int(const std::string value);

double                   parse_number(const std::string value);

std::vector<std::string> split(const std::string string, const std::string delimeter);

void                     split(std::vector<std::string>& target, const std::string source, const std::string delimeter);

std::vector<std::string> tokens(const std::string string);

void                     tokens(std::vector<std::string>& target, const std::string source);

std::string              tolowerstr(std::string string);

std::string              toupperstr(std::string string);

std::string              trim(const std::string string);

std::string              trim_end(const std::string string);

#endif /* util_h */
