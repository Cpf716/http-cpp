//
//  json.cpp
//  http
//
//  Created by Corey Ferguson on 9/2/25.
//

#include "json.h"

namespace json {
    // Constructors

    array::array() {
        this->type() = json::type::array;
    }
        
    array::array(const std::string key): array() {
        this->_key = key;
    }

    array::array(const size_t size): array() {
        this->values().resize(size);

        for (size_t i = 0; i < this->size(); i++)
            this->values()[i] = new object();
    }

    array::array(const std::vector<object*> values): array() {
        for (object* value: values) this->set(value);
    }

    error::error(const std::string what) {
        this->_what = what;
    }

    array::iterator::iterator(const size_t size, std::vector<object*> values) {
        this->_size = size;
        this->_values = values;
    }

    object::object() { }

    object::object(const std::string key) {
        this->_key = key;
    }

    object::object(const enum type type) {
        this->type() = type;
    }

    object::object(const std::vector<object*> values) : object(json::type::object) {
        for (object* value: values) this->set(value);
    }

    object::object(const std::string key, const enum type type) : object(key) {
        this->type() = type;
    }

    object::object(const std::string key, const std::string value) : object(key) {
        this->_value = value;
    }

    object::~object() {
        for (object* value: this->values()) delete value;
    }

    // Operators

    object* array::iterator::operator*() const {
        return this->_values[this->_index];
    }

    array::iterator& array::iterator::operator+(int value) {
        this->_index += value;

        if (this->_index > this->_size)
            this->_index = (int) this->_size;
        else if (this->_index < 0)
            this->_index = 0;

        return *this;
    }

    array::iterator& array::iterator::operator++() {
        if (this->_index != this->_size)
            this->_index++;

        return *this;
    }

    array::iterator& array::iterator::operator++(int) {
        array::iterator& temp = *this;

        if (this->_index != this->_size)
            this->_index++;

        return temp;
    }

    array::iterator& array::iterator::operator-(int value) {
        this->_index -= value;

        if (this->_index < 0)
            this->_index = 0;
        else if (this->_index > this->_size)
            this->_index = (int) this->_size;

        return *this;
    }

    array::iterator& array::iterator::operator--() {
        if (this->_index != 0)
            this->_index--;

        return *this;
    }

    array::iterator& array::iterator::operator--(int) {
        array::iterator& temp = *this;

        if (this->_index != 0)
            this->_index--;

        return temp;
    }

    bool array::iterator::operator==(const array::iterator& value) const {
        return this->_index == value._index;
    }

    bool array::iterator::operator!=(const array::iterator& value) const {
        return this->_index != value._index;
    }

    // Member Functions

    void object::_erase(const std::string key) {
        auto it = this->_map.find(key);

        delete this->values()[this->size() + (* it).second];

        this->values().erase(this->values().begin() + this->size() + (* it).second);
        this->_map.erase(it);
    }

    int object::_find(const std::string key) {
        auto it = this->_map.find(key);

        return it == this->_map.end() ? -1 : (int) (* it).second;
    }

    void object::_set(const size_t index, object* value) {
        value->_key = "";
                    
        // Replace item
        if (index < this->size())
            this->values()[index] = value;
            // Add item
        else {
            while (this->size() < index) {
                this->values().push_back(new object());
                
                // Sort before named values
                for (size_t i = 0; i < this->_map.size(); i++)
                    std::swap(this->values()[this->values().size() - i - 1], this->values()[this->values().size() - i - 2]);
            }
            
            this->values().push_back(value);
            
            // Sort before named values
            for (size_t i = 0; i < this->_map.size(); i++)
                std::swap(this->values()[this->values().size() - i - 1],this->values()[this->values().size() - i - 2]);
        }
    }

    json::array* array::_splice(const int start, const int delete_count, const std::vector<object*> values) {
        json::array* result = new json::array();

        for (int i = 0; i < delete_count; i++) {
            result->set(this->values()[start]);

            this->values().erase(this->values().begin() + start);
        }

        for (size_t i = 0; i < values.size(); i++) {
            if (values[i]->key().length())
                // Cannot splice named properties
                throw error("Operation not permitted");

            this->values().insert(this->values().begin() + start + i, values[i]);
        }
        
        return result;
    }

    object* object::assign(object* value) {
        if (this->null()) throw error(json::null);
        if (this->undefined()) throw error(json::undefined);

        if (value->type() == json::type::primitive && (value->value().empty() || value->value() == json::undefined))
            return this;

        // Target is an array; clear its items
        if (this->type() == json::type::array) {
            if (value->type() == json::type::array) {
                for (size_t i = 0; i < this->size() && i < value->size(); i++) {
                    delete this->values()[i];

                    ((json::array *)this)->set(i, parse(value->values()[i]->str()));
                }

                for (size_t i = this->size(); i < value->size(); i++)
                    ((json::array *)this)->set(i, parse(value->values()[i]->str()));
                // Source is an object; do nothing
            }
        // Target is an object
        } else {
            if (this->type() != json::type::object)
                throw error("Operation not permitted");
            
            // Source is an array; assign its items' keys by index
            for (size_t i = 0; i < value->size(); i++) {
                object* temp = parse(((array *)value)->get(i)->str());

                temp->_key = std::to_string(i);

                this->set(temp);
            }

            for (size_t i = value->size(); i < value->values().size(); i++) {
                object* temp = parse(value->values()[i]->str());

                temp->_key = value->values()[i]->key();

                this->set(temp);
            }
        }
        
        return this;
    }

    object* array::at(int index) {
        if (index < 0) {
            index += this->size();

            if (index < 0)
                return NULL;
        } else if (index >= this->size())
            return NULL;

        return this->get(index);
    }

    array::iterator array::begin() {
        return array::iterator(this->size(), this->values());
    }

    json::array* array::concat(std::vector<object*> values) {
        json::array* result = new json::array(this->values());

        for (object* value: values)
            for (size_t i = 0; i < value->size(); i++)
                result->set(((json::array *)value)->get(i));
        
        return result;
    }

    array::iterator array::end() {
        return this->begin() + (int) this->size();
    }

    void object::erase() {
        this->type() = json::type::primitive;
        this->value().clear();
        this->_map.clear();

        // Note: Values must be explicitly deallocated
        this->values().clear();
    }

    void object::erase(const size_t index) {
        this->erase(std::to_string(index));
    }

    void object::erase(const std::string key) {
        if (this->type() == json::type::array) {
            int index = parse_int(key);
            
            // Named item
            if (index == INT_MIN)
                this->_erase(key);
            // Anonymous item
            else if (index < this->size())
                ((array *) this)->get(index)->erase();
            // (Named) property
        } else {
            if (this->type() != json::type::object)
                throw error("Operation not permitted");

            this->_erase(key);
        }
    }

    object* array::get(const size_t index) {
        return object::get(std::to_string(index));
    }

    object* array::get(std::string key) {
        return this->object::get(key);
    }

    object* object::get(const std::string key) {
        if (this->type() == json::type::array) {
            int index = parse_int(key);
            
            if (index == INT_MIN) {
                index = this->_find(key);
                
                if (index == -1)
                    return NULL;
                
                return this->values()[this->size() + index];
            } else {
                if (index < 0) {
                    index = this->_find(key);
                    
                    if (index == -1)
                        return NULL;
                    
                    return this->values()[this->size() + index];
                }
                
                if (index < this->size())
                    return this->values()[index];
                
                return NULL;
            }
        }
        
        if (this->type() != json::type::object)
            throw error("Operation not permitted");
        
        int index = this->_find(key);
        
        if (index == -1)
            return NULL;
        
        return this->values()[this->size() + index];
    }

    std::string object::key() const {
        return this->_key;
    }

    std::vector<std::string> object::keys() {
        std::vector<std::string> result;

        if (this->type() == json::type::array) {
            result.reserve(this->values().size());

            // Push item indices
            size_t i;

            for (i = 0; i < this->size(); i++)
                result.push_back(std::to_string(i));

            // Push property keys
            for (; i < this->values().size(); i++)
                result.push_back(this->values()[i]->key());
        } else if (this->type() == json::type::object) {
            result.reserve(this->values().size());

            // Push property keys
            for (object* o: this->values())
                result.push_back(o->key());
        }

        return result;
    }

    void object::map() {
        if (this->_map.size()) return;

        size_t i;

        for (i = 0; i < this->values().size() && this->values()[i]->key().empty(); i++)
            this->values()[i]->map();

        for (; i < this->values().size(); i++) {
            if (this->values()[i]->key().empty())
                throw error(json::undefined);

            this->_map.emplace(this->values()[i]->key(), i);
            this->values()[i]->map();
        }

        if (this->type() == json::type::object && this->size())
            // Objects cannot have anonymous properties
            throw error("Operation not permitted");
    }

    bool object::null() {
        return this->type() == json::type::primitive && this->value() == json::null;
    }

    double object::number() {
        return parse_number(this->value());
    }

    void object::nullify() {
        this->type() = json::type::primitive;
        this->value() = json::null;

        this->_map.clear();
        
        for (object* value: this->values()) delete value;

        this->values().clear();
    }

    std::string object::pretty() {
        if (this->null()) throw error(json::null);
        if (this->undefined()) throw error(json::undefined);

        std::stack<object*> stack;
        std::string         result;

        stack.push(this);

        object* previous = NULL;

        std::stack<object*> parents;

        int indent = 0;

        while (!stack.empty()) {
            object* current = stack.top();

            switch (current->type()) {
                case json::type::array:
                case json::type::object: {
                    auto delimeter =
                        ((std::map<json::type, std::pair<std::string, std::string>>) {
                            { json::type::array, { "[", "]" }},
                            { json::type::object, { "{", "}" }}
                        })[current->type()];

                    if (current->values().empty()) {
                        if (current->value().length())
                            throw error("Operation not permitted");

                        result.append(indent * 2, ' ');
                        
                        if (!parents.empty() && current->key().length()) {
                            result += escape(current->key());
                            result += ":";
                            result += " ";
                        }
                        
                        result += delimeter.first;
                        result += delimeter.second;
                        
                        if (!(parents.empty() || current == parents.top()->values().back()))
                            result += ",";

                        result += "\n";

                        stack.pop();

                        previous = current;
                    } else if (previous == current->values().back()) {
                        result.append((indent - 1) * 2, ' ');

                        parents.pop();
                        
                        result += delimeter.second;
                        
                        if (!(parents.empty() || current == parents.top()->values().back()))
                            result += ",";

                        result += "\n";

                        stack.pop();

                        previous = current;
                        indent--;
                    } else {
                        result.append(indent * 2, ' ');

                        if (!parents.empty() && current->key().length()) {
                            result += escape(current->key());
                            result += ":";
                            result += " ";
                        }
                        
                        result += delimeter.first;
                        result += "\n";
                        
                        // Push children onto stack
                        for (int i = (int) current->values().size(); i > 0; i--)
                            stack.push(current->values()[i - 1]);
                        
                        parents.push(current);

                        indent++;
                    }
                    break;
                } case json::type::primitive: {
                    if (current->values().size())
                        throw error("Operation not permitted");

                    result.append(indent * 2, ' ');

                    if (!parents.empty() && current->key().length()) {
                        result += escape(current->key());
                        result += ":";
                        result += " ";
                    }
            
                    result += current->value().empty() ? json::undefined : current->value();
                    
                    if (!(parents.empty() || current == parents.top()->values().back()))
                        result += ",";

                    result += "\n";

                    stack.pop();

                    previous = current;
                    break;
                } default:
                    break;
            }
        }

        return result;
    }

    object* object::sanitize() {
        size_t i = this->size();

        while (i < this->values().size()) {
            if (this->values()[i]->undefined()) {
                this->_erase(
                    this->values()[i]->key()
                );
            } else {
                this->values()[i]->sanitize();
                i++;
            }
        }
        
        return this;
    }

    object* array::set(object* value) {
        return this->object::set(value);
    }

    object* object::set(object* value) {
        if (this->type() == json::type::array) {
            if (value->key().empty()) {
                this->values().push_back(value);
                
                // Sort before named values
                for (size_t i = 0; i < this->_map.size(); i++)
                    std::swap(this->values()[this->values().size() - i - 1],this->values()[this->values().size() - i - 2]);
            } else {
                int index = parse_int(value->key());
                
                // Named value
                if (index == INT_MIN) {
                    this->values().push_back(value);
                    this->_map.emplace(value->key(), this->_map.size());
                } else if (index >= 0)
                    this->_set(index, value);
                else {
                    this->values().push_back(value);
                    this->_map.emplace(value->key(), this->_map.size());
                }
            }
            
            return value;
        }

        if (this->type() != json::type::object)
            throw error("Operation not permitted");

        if (value->key().empty())
            // Objects cannot have anonymous properties
            throw error("Operation not permitted");
        
        int index = this->_find(value->key());
        
        if (index == -1) {
            this->values().push_back(value);
            this->_map.emplace(value->key(), this->_map.size());
        } else
            this->values()[this->size() + index] = value;
        
        return value;
    }

    object* array::set(const size_t index, object* value) {
        this->_set(index, value);

        return value;
    }

    size_t object::size() {
        return this->values().size() - this->_map.size();
    }

    json::array* array::slice(const int start) {
        return this->slice(start, (int) this->size());
    }

    json::array* array::slice(int start, int end) {
        json::array* result = new json::array();

        if (start < 0) {
            start += this->size();

            if (start < 0)
                start = 0;
        }

        if (end < 0) {
            end += this->size();

            if (end < 0)
                end = 0;
        } else if (end > this->size())
            end = (int) this->size();

        for (int i = start; i < end; i++)
            result->set(this->get(i));

        return result;
    }

    json::array* array::splice(int start) {
        int delete_count;

        if (start < 0) {
            start += (int) this->size();
            delete_count = (int) this->size();

            if (start < 0)
                start = 0;
            else
                delete_count -= start;
        } else if (start >= this->size())
            delete_count = 0;
        else
            delete_count = (int) this->size() - start;

        return this->_splice(start, delete_count, std::vector<object*>());
    }

    json::array* array::splice(const int start, const int delete_count) {
        return this->splice(start, delete_count, std::vector<object*>());
    }

    json::array* array::splice(int start, int delete_count, const std::vector<object*> values) {
        if (this->type() != json::type::array)
            throw error("Operation not permitted");

        if (start < 0) {
            start += (int) this->size();
            delete_count = (int) this->size();

            if (start < 0)
                start = 0;
            else
                delete_count -= start;
        } else if (start >= this->size())
            delete_count = 0;
        else if (start + delete_count > this->size())
            delete_count = (int) this->size() - start;

        return this->_splice(start, delete_count, values);
    }

    std::string object::str() {
        if (this->null()) throw error(json::null);
        if (this->undefined()) throw error(json::undefined);

        std::stack<object*> stack;
        std::string         result;

        stack.push(this);

        object* previous = NULL;

        std::stack<object*> parents;

        while (!stack.empty()) {
            object* current = stack.top();

            switch (current->type()) {
                case json::type::array:
                case json::type::object: {
                    auto delimeter =
                        ((std::map<json::type, std::pair<std::string, std::string>>) {
                            { json::type::array, { "[", "]" }},
                            { json::type::object, { "{", "}" }}
                        })[current->type()];

                    if (current->values().empty()) {
                        if (current->value().length())
                            throw error("Operation not permitted");
                        
                        if (!parents.empty() && current->key().length()) {
                            result += escape(current->key());
                            result += ":";
                        }
                        
                        result += delimeter.first;
                        result += delimeter.second;
                        
                        if (!(parents.empty() || current == parents.top()->values().back()))
                            result += ",";

                        stack.pop();

                        previous = current;
                    } else if (previous == current->values().back()) {
                        parents.pop();
                        
                        result += delimeter.second;
                        
                        if (!(parents.empty() || current == parents.top()->values().back()))
                            result += ",";

                        stack.pop();

                        previous = current;
                    } else {
                        if (!parents.empty() && current->key().length()) {
                            result += escape(current->key());
                            result += ":";
                        }
                        
                        result += delimeter.first;
                        
                        // Push children onto stack
                        for (int i = (int) current->values().size(); i > 0; i--)
                            stack.push(current->values()[i - 1]);
                        
                        parents.push(current);
                    }
                    break;
                } case json::type::primitive: {
                    if (current->values().size())
                        throw error("Operation not permitted");

                    if (!parents.empty() && current->key().length()) {
                        result += escape(current->key());
                        result += ":";
                    }
            
                    result += current->value().empty() ? json::undefined : current->value();
                    
                    if (!(parents.empty() || current == parents.top()->values().back()))
                        result += ",";

                    stack.pop();

                    previous = current;
                    break;
                } default:
                    break;
            }
        }

        return result;
    }

    std::string object::str_value() {
        return this->value();
    }

    enum json::type& object::type() {
        return this->_type;
    }

    bool object::undefined()  {
        return this->type() == json::type::primitive && this->value().empty();
    }

    std::string& object::value() {
        return this->_value;
    }

    std::vector<object*>& object::values() {
        return this->_values;
    }

    const char* error::what() const throw() {
        return this->_what.c_str();
    }

    // Non-Member Functions

    object* from_value(const double value) {
        object* result = new object();

        result->value() = truncate_d(value);

        return result;
    }

    object* from_value(const std::string value) {
        object* result = new object();

        result->value() = value;

        return result;
    }

    object* parse(object* target, const std::string_view source, const int start, const int end) {
        int i = start;

        while (i < end && isspace(source[i])) i++;

        if (source[i] == ',') throw error("Unexpected token , in JSON");
        if (source[i] == '{') {
            i++;

            // Find closing curly brace
            int  j,
                  p = 1;
            bool escape = false;
                    
            for (j = i; j < end; j++) {
                if (source[j] == '\"' && (j == i || source[j - 1] != '\\'))
                    escape = !escape;
                else if (!escape) {
                    if (source[j] == '{')
                        p++;
                    else if (source[j] == '}') {
                        if (p == 1) break;

                        p--;
                    }
                }
            }

            if (j == end)
                throw error("Unexpected end of JSON input");

            target->type() = json::type::object;

            // Parse properties
            int k = i;

            while (k < j) {
                if (isspace(source[k]))
                    k++;
                else if (source[k] == ',') {
                    if (k == i || k == j - 1 || source[k + 1] == ',')
                        throw error("Unexpected token , in JSON");

                    k++;
                } else {
                    // Find comma/next item
                    int l;

                    p = 0;

                    escape = false;

                    for (l = k; l < j; l++) {
                        if (source[l] == '\"' && (l == k || source[l - 1] != '\\'))
                            escape = !escape;
                        else if (source[l] == '{' || source[l] == '[')
                            p++;
                        else if (source[l] == '}' || source[l] == ']')
                            p--;
                        else if (!escape && !p &&  source[l] == ',')
                            break;
                    }

                    // Find colon/key
                    int m;

                    p = 0;

                    escape = false;

                    for (m = k; m < l; m++) {
                        if (source[m] == '\"' && (m == k || source[m - 1] != '\\'))
                            escape = !escape;
                        else if (source[m] == '{' || source[m] == '[')
                            p++;
                        else if (source[m] == '}' || source[m] == ']')
                            p--;
                        else if (!escape && !p && source[m] == ':')
                            break;
                    }

                    if (m == l) throw error("Expected ':'");

                    std::string key = ::unescape(std::string(source.substr(k, m - k)));

                    object* property = new object(key);

                    m++;

                    target->values().push_back(parse(property, source, m, l));

                    k = l;
                }
            }
            // Parse array
        } else if (source[i] == '[') {
            i++;

            // Find closing square bracket
            int  j,
                  p = 1;
            bool escape = false;
                    
            for (j = i; j < end; j++) {
                if (source[j] == '\"' && (j == i || source[j - 1] != '\\'))
                    escape = !escape;
                else if (!escape) {
                    if (source[j] == '[')
                        p++;
                    else if (source[j] == ']') {
                        if (p == 1) break;

                        p--;
                    }
                }
            }

            if (j == end)
                throw error("Unexpected end of JSON input");

            target->type() = json::type::array;
            
            // Parse items
            int k = i;

            while (k < j) {
                if (isspace(source[k]))
                    k++;
                else if (source[k] == ',') {
                    if (k == i || k == j - 1 || source[k + 1] == ',')
                        throw error("Unexpected token , in JSON");

                    k++;
                } else {
                    // Find comma/next item
                    int l;

                    p = 0;

                    escape = false;

                    for (l = k; l < j; l++) {
                        if (source[l] == '\"' && (l == k || source[l - 1] != '\\'))
                            escape = !escape;
                        else if (source[l] == '{' || source[l] == '[')
                            p++;
                        else if (source[l] == '}' || source[l] == ']')
                            p--;
                        else if (!escape && !p &&  source[l] == ',')
                            break;
                    }

                    // Find colon/key
                    int m;

                    p = 0;

                    escape = false;

                    for (m = k; m < l; m++) {
                        if (source[m] == '\"' && (m == k || source[m - 1] != '\\'))
                            escape = !escape;
                        else if (source[m] == '{' || source[m] == '[')
                            p++;
                        else if (source[m] == '}' || source[m] == ']')
                            p--;
                        else if (!escape && !p && source[m] == ':')
                            break;
                    }

                    std::string key;

                    if (m == l) m = k;
                    else {
                        if (l == m + 1)
                            throw error("Unexpected token ':'");

                        key = ::unescape(std::string(source.substr(k, m - k)));
                        m++;
                    }

                    object* item = new object(key);

                    target->values().push_back(parse(item, source, m, l));

                    k = l;
                }
            }
            // Parse primitive
        } else {
            if (source[i] == '}') throw error("Unexpected token '}'");
            if (source[i] == ']') throw error("Unexpected token ']'");

            // Find comma/next property
            target->type() = json::type::primitive;

            std::string_view value = trim(source.substr(i, end - i));

            if (value != json::undefined)
                target->value() = std::string(value);
        }
        
        return target;
    }

    object* parse(const std::string text) {
        if (text.empty())
            throw error("Unexpected end of JSON input");
        
        object* o = new object();

        parse(o, text, 0, (int) text.length());

        o->map();

        return o;
    }

    std::string typestr(object* value) {
        switch (value->type()) {
            case json::type::array:
                return "array";
            case json::type::object:
                return "object";
            case json::type::primitive: {
                if (value->value() == null)
                    return "unknown";

                std::string lowerstr = tolowerstr(value->value());

                if (lowerstr == "true" || lowerstr == "false")
                    return "boolean";

                return is_number(value->value()) ? "number" : "string";
            }
        }
    }
}
