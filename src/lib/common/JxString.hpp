#pragma once

#include <ostream>
#include <string>

class JxString {
public:
    JxString();
    JxString(char const* beg);
    JxString(char const* beg, char const* end);

    JxString& operator=(JxString const& rhs);
    void clear();


    void assign(char const* beg, char const* end);
    size_t size() const;
    bool empty() const;

    char operator[](size_t idx) const;

    bool startsWith(char const* s) const;
    bool startsWith(std::string const& s) const;
    bool operator==(JxString const& rhs) const;
    bool operator==(std::string const& rhs) const;
    bool operator==(char const* rhs) const;

    char const* begin() const;
    char const* end() const;

protected:
    char const* _beg;
    char const* _end;
    size_t _size;
};

bool operator==(std::string const& lhs, JxString const& rhs);
bool operator==(char const* lhs, JxString const& rhs);
std::ostream& operator<<(std::ostream& stream, JxString const& str);
