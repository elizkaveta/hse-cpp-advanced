#pragma once

#include <string>
#include <cstring>
#include <iostream>

class StringView {
public:
    StringView(const std::string& s, size_t begin, size_t end);
    StringView(const char* s);
    StringView(const char* s, size_t size);
    const char& operator[](size_t i) const;
    size_t Size() const;

private:
    size_t size_ = 0;
    const char* begin_;
};

StringView::StringView(const std::string& s, size_t begin = 0, size_t end = std::string::npos) {
    begin_ = s.c_str() + begin;
    size_ = std::min(end, s.size() - begin);
}

StringView::StringView(const char* s) {
    begin_ = s;
    size_ = std::strlen(s);
}

StringView::StringView(const char* s, size_t size) {
    begin_ = s;
    size_ = size;
}

const char& StringView::operator[](size_t i) const {
    if (i >= size_) {
        throw std::runtime_error("index out of range");
    }
    return *(begin_ + i);
}

size_t StringView::Size() const {
    return size_;
}
