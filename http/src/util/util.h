//
//  util.h
//  http
//
//  Created by Corey Ferguson on 1/28/26.
//

#ifndef util_h
#define util_h

#include <iostream>

// Non-Member Functions

// Return the digits comprising a floating-point number; the decimal point is represented by INT_MAX
std::vector<int>         digits(const double number);

/**
 * Return string escaped by double quotations
 */
std::string              escape(const std::string string);

bool                     is_int(const std::string value);

bool                     is_number(const std::string value);


// Return true if b is a power of n
bool                     is_pow(const size_t b, const size_t n);

std::string              join(std::vector<std::string> values, std::string delimeter);

int                      parse_int(const std::string value);

double                   parse_number(const std::string value);

/**
 * Return the next power of n for b
 * I.e. pow(15, 2) = 16
 */
int                      pow(const int b, const int n = 2);

std::vector<std::string> split(const std::string string, const std::string delimeter);

void                     split(std::vector<std::string>& target, const std::string source, const std::string delimeter);

std::vector<std::string> tokens(const std::string string);

void                     tokens(std::vector<std::string>& target, const std::string source);

std::string              tolowerstr(std::string string);

std::string              toupperstr(std::string string);

std::string              trim(const std::string string);

std::string_view         trim(const std::string_view string);

std::string              trim_end(const std::string string);

// Return floating-point number formatted to min precision
std::string              truncate_d(const double number, const int minprec = 0);

/**
 * Unescape double quotation-escaped string
 */
std::string              unescape(const std::string string);

#endif /* util_h */
