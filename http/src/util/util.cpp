//
//  util.cpp
//  http
//
//  Created by Corey Ferguson on 1/28/26.
//

#include "util.h"

// 1. (\+|-)?
// 2. (\+|-)?[0-9]+
bool is_int(const std::string value) {
    int i = 0;
    
    if (i != value.length() && (value[i] == '+' || value[i] == '-'))
        i++;
    
    if (i == value.length())
        return false;
    
    for (; i < value.length(); i++)
        if (!isdigit(value[i]))
            return false;
    
    return true;
}

// 1. (\+|-)?
// 2. (\+|-)?[0-9]+(\.[0-9]+)?
// 3. (\+|-)?([0-9]+(\.[0-9]+)?|[0-9]*\.[0-9]+)((E|e)(\+|-)?[0-9]+)?
bool is_number(const std::string value) {
    if (value.empty())
        return false;
    
    int i = 0;
    
    // Leading positive (+) or negative (-) sign
    if (value[i] == '+' || value[i] == '-')
        i++;
    
    // Find decimal point
    int j = i;
    
    while (j < value.length() && value[j] != '.')
        j++;
    
    // If no decimal point is found, start at the beginning (after the sign, if applicable)
    // Find exponent
    int k = j == value.length() ? i : j;
    
    while (k < value.length() && !(value[k] == 'E' || value[k] == 'e'))
        k++;
    
    // Stop at the decimal point, if applicable; otherwise stop at the exponent, if applicable
    int l = j < k ? j : k,
        m = i;
    
    for (; m < l; m++)
        if (!isdigit(value[m]))
            return false;
    
    // Count the number of digits between the beginning (after sign, if applicable) and the decimal point (if applicable)
    // and the decimal point (if applicable) and the exponent (if applicable)
    size_t n = l - i;
    
    //  After decimal (if applicable) and before exponent (if applicable)
    if (j != value.length()) {
        for (m = j + 1; m < k; m++)
            if (!isdigit(value[m]))
                return false;
        
        n += k - j - 1;
    }
    
    // There are no digits between sign (if applicable) and decimal point (if applicable)
    // and/or decimal point (if applicable) and exponent (if applicable)
    if (n == 0)
        return false;
    
    // After exponent (if applicable)
    if (k != value.length()) {
        size_t l = k + 1;
        
        if (l == value.length())
            return false;
        
        // Leading positive (+) or negative (-) sign
        if (value[l] == '+' || value[l] == '-')
            l++;
        
        if (l == value.length())
            return false;
        
        for (; l < value.length(); l++)
            if (!isdigit(value[l]))
                return false;
        // Single digit
    }
    
    return true;
}

bool is_pow(const size_t b, const size_t n) {
    if (b == 0)
        return false;
    
    if (n == 0)
        return b < 2;
    
    int result = log(b) / log(n);
    
    return (int) result - result == 0;
}

std::string join(std::vector<std::string> values, std::string delimeter) {
    std::string result;
        
    if (values.size()) {
        for (size_t i = 0; i < values.size() - 1; i++) {
            result += values[i];
            result += delimeter;
        }

        result += values[values.size() - 1];
    }
        
    return result;
}

int parse_int(const std::string value) {
    return is_int(value) ? stoi(value) : INT_MIN;
}

double parse_number(const std::string value) {
    return is_number(value) ? stod(value) : NAN;
}

std::vector<std::string> split(const std::string string, const std::string delimeter) {
    int start = 0;
    
    std::vector<std::string> result;

    result.reserve(string.length() + 1);

    for (int end = 0; end <= (int) string.length() - (int) delimeter.length(); end++) {
        int index = 0;

        while (index < delimeter.length() && string[end + index] == delimeter[index])
            index++;
        
        if (index == delimeter.length()) {
            result.push_back(string.substr(start, end - start));

            start = end + index;
        }
    }
    
    result.push_back(string.substr(start));

    return result;
}

void split(std::vector<std::string>& target, const std::string source, const std::string delimeter) {
    int start = 0;

    for (int end = 0; end <= (int) source.length() - (int) delimeter.length(); end++) {
        int index = 0;

        while (index < delimeter.length() && source[end + index] == delimeter[index])
            index++;
        
        if (index == delimeter.length()) {
            target.push_back(source.substr(start, end - start));

            start = end + index;
        }
    }
    
    target.push_back(source.substr(start));
}

std::vector<std::string> tokens(const std::string string) {
    std::vector<std::string> result;

    result.reserve(string.length() + 1);
    
    for (size_t start = 0, end = 0; end < string.length(); end++) {
        while (end < string.length() && isspace(string[end]))
            end++;
        
        start = end;
        
        while (end < string.length() && !isspace(string[end]))
            end++;
        
        if (start != end)
            result.push_back(string.substr(start, end - start));
    }

    return result;
}

void tokens(std::vector<std::string>& target, const std::string source) {
    target.reserve(source.length());

    for (size_t start = 0, end = 0; end < source.length(); end++) {
        while (end < source.length() && isspace(source[end]))
            end++;
        
        start = end;
        
        while (end < source.length() && !isspace(source[end]))
            end++;
        
        if (start != end)
            target.push_back(source.substr(start, end - start));
    }
}

std::string tolowerstr(std::string string) {
    std::transform(string.begin(), string.end(), string.begin(), ::tolower);

    return string;
}

std::string toupperstr(std::string string) {
    std::transform(string.begin(), string.end(), string.begin(), ::toupper);

    return string;
}

std::string trim(const std::string string) {
    // Find leading whitespace
    size_t start = 0;
    
    while (start < string.length() && isspace(string[start]))
        start++;
    
    // Find trailing whitespace
    size_t end = string.length();
    
    while (end > start && isspace(string[end - 1]))
        end--;
        
    return string.substr(start, end - start);
}

std::string trim_end(const std::string string) {
    size_t end = string.length();

    while (end > 0 && isspace(string[end - 1]))
        end--;
        
    return string.substr(0, end);
}
