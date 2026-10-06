//
//  json.h
//  http
//
//  Created by Corey Ferguson on 9/2/25.
//

#ifndef json_h
#define json_h

#include "util.h"
#include <map>

namespace json {
    enum class type { array, object, primitive };

    // Non-Member Fields

    static const std::string null = "null";

    static const std::string undefined = "undefined";

    // Typedef
    
    class error: public std::exception {
        std::string _what;
    public:
        // Constructors

        error(const std::string what);

        // Member Fields

        const char* what() const throw();
    };

    class object {
        // Member Fields
        
        std::unordered_map<std::string, size_t> _map;
        enum type                               _type = json::type::primitive;
        std::string                             _value;
        
        // Member Functions

        void _erase(const std::string key);

        /**
         * Perform hash lookup and return the property index, otherwise -1 if it does not exist
         */
        int  _find(const std::string key);
    protected:
        // Member Fields

        std::string          _key;
        std::vector<object*> _values;

        // Member Functions

        void _set(const size_t index, object* value);
    public:
        // Typedef
        
        // Constructors
        
        object();
        
        object(const std::string key);

        object(const enum type type);

        object(const std::vector<object*> values);
        
        object(const std::string key, const enum type type);
        
        object(const std::string key, const std::string value);

        ~object();

        // Member Functions

        /**
         * Deep copy source and assign its contents to target
         */
        object*                  assign(object* value);

        /**
         * Set undefined
         */
        void                     erase();

        /**
         * Set item undefined
         */
        void                     erase(const size_t index);
        
        /**
         * Delete property
         */
        void                     erase(const std::string key);

        /**
         * Return property if it exists, otherwise return NULL
         */
        object*                  get(std::string key);

        std::string              key() const;

        std::vector<std::string> keys();

        /**
         * Build key map
         */
        void                     map();

        bool                     null();

        void                     nullify();

        double                   number();

        std::string              pretty();
                
        /**
         * Delete undefined properties
         */
        object*                  sanitize();

        /**
         * Set array item or object property and return it
         */
        object*                  set(object* value);
        
        /**
         * Return array size
         */
        size_t                   size();

        std::string              str();

        std::string              str_value();

        enum type&               type();
        
        bool                     undefined();

        std::string&             value();

        // Do not modify!
        std::vector<object*>&    values();    
    };

    class array: public object {
        json::array* _splice(const int start, const int delete_count, const std::vector<object*> values);
    public:
        // Typedef

        class iterator {
            // Member Fields

            int                  _index = 0;
            size_t               _size;
            std::vector<object*> _values;

            // Constructors

            iterator(const size_t size, std::vector<object*> values);
        public:
            // Typdef

            friend json::array;

            // Operators

            object*   operator*() const;

            // object*   operator->() const;

            iterator& operator+(int value);

            iterator& operator++();

            iterator& operator++(int);

            iterator& operator-(int value);

            iterator& operator--();

            iterator& operator--(int);

            bool      operator==(const iterator& value) const;

            bool      operator!=(const iterator& value) const;
        };

        // Constructors
        
        array();
        
        array(const std::string key);

        array(const size_t size);

        array(const std::vector<object*> values);

        // Member Functions

        object*      at(const int index);

        iterator     begin();

        json::array* concat(std::vector<object*> values);

        iterator     end();

        /**
         * Return property if it exists, otherwise return NULL
         */
        object*      get(std::string key);

        /**
         * Return value at index if it exists, otherwise return NULL
         */
        object*      get(const size_t index);

        /**
         * Set array item or object property and return it
         */
        object*      set(object* value);

        /**
         * Shorthand for array items
         */
        object*      set(const size_t index, object* value);

        json::array* slice(const int start);

        json::array* slice(int start, int end);

        json::array* splice(int start);

        json::array* splice(const int start, const int delete_count);
        
        json::array* splice(int start, int delete_count, const std::vector<object*> values);
    };

    // Non-Member Functions

    object*                                      from_value(const double value);

    object*                                      from_value(const std::string value);

    object*                                      parse(const std::string text);

    std::string                                  typestr(object* value);
}

#endif /* json_h */
