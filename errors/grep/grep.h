#pragma once

#include <optional>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "utf8.h"  // default utf8 libary
#include <cstddef>
#include <functional>

using std::optional;
using namespace std::filesystem;
struct GrepOptions {
    optional<size_t> look_ahead_length;
    size_t max_matches_per_line;

    GrepOptions() {
        max_matches_per_line = 10;
    }

    GrepOptions(size_t look_ahead_length) : GrepOptions() {
        this->look_ahead_length = look_ahead_length;
    }

    GrepOptions(optional<size_t> look_ahead_length, size_t max_matches_per_line) {
        this->look_ahead_length = look_ahead_length;
        this->max_matches_per_line = max_matches_per_line;
    }
};

std::string ConvertFile(std::string::iterator last_word, std::string::iterator end, int len,
                        size_t sz) {
    while (sz) {
        ++last_word;
        --sz;
    }
    if (len == -1) {
        return std::string(last_word, end);
    } else {
        std::string::iterator end_iterator;
        end_iterator = last_word;
        for (size_t i = 0; static_cast<int>(i) < len; ++i) {
            ++end_iterator;
            while (end_iterator != end && (*end_iterator & 192) == 128) {
                ++end_iterator;
            }
        }
        return std::string(last_word, end_iterator);
    }
}
template <class Visitor>
void GetLine(std::string::iterator& last_match, std::string& line, size_t line_number,
             const std::string& path, Visitor visitor, size_t max_count, const std::string& pattern,
             int left_cnt) {
    size_t current_count = 0;
    while (current_count < max_count && last_match != line.end()) {
        ++current_count;
        if (left_cnt == -1) {
            visitor.OnMatch(path, line_number, utf8::distance(line.begin(), last_match) + 1,
                            std::nullopt);
        } else {
            if (left_cnt > utf8::distance(last_match, line.end()) - 2) {
                visitor.OnMatch(path, line_number, utf8::distance(line.begin(), last_match) + 1,
                                ConvertFile(last_match, line.end(), -1, pattern.size()));
            } else {
                visitor.OnMatch(path, line_number, utf8::distance(line.begin(), last_match) + 1,
                                ConvertFile(last_match, line.end(), left_cnt, pattern.size()));
            }
        }
        last_match = std::search(++last_match, line.end(),
                                 std::default_searcher(pattern.begin(), pattern.end()));
    }
}

template <class Visitor>
void GetFile(const std::string& path, const std::string& pattern, Visitor visitor,
             const GrepOptions& options, std::ifstream& file) {
    size_t cur_line = 1;
    std::string line;
    int left_cnt = -1;
    if (options.look_ahead_length != std::nullopt) {
        left_cnt = options.look_ahead_length.value();
    }
    while (std::getline(file, line)) {
        std::string::iterator it = std::search(
            line.begin(), line.end(), std::default_searcher(pattern.begin(), pattern.end()));
        GetLine(it, line, cur_line++, path, visitor, options.max_matches_per_line, pattern,
                left_cnt);
    }
}

bool Valid(const std::string& path) {
    std::ifstream is(path);
    if (!is.is_open()) {
        return 1;
    }
    if (status(path).type() == file_type::symlink) {
        return 1;
    }
    std::istreambuf_iterator<char> x(is.rdbuf());
    if (*x == '\0') {
        return 1;
    }
    std::istreambuf_iterator<char> f;
    return !utf8::is_valid(x, f);
}

template <class Visitor>
static void GetFile(const std::string& path, const std::string& pattern, Visitor visitor,
                    const GrepOptions& options) {
    std::ifstream is(path);
    if (Valid(path)) {
        visitor.OnError("is " + path + " is not valid");
        return;
    }
    GetFile(path, pattern, visitor, options, is);
}

template <class Visitor>
void Grep(const std::string& path, const std::string& pattern, Visitor visitor,
          const GrepOptions& options) {
    if (is_directory(path)) {
        for (auto const& dir_entry : directory_iterator(path)) {
            if (is_directory(dir_entry.path())) {
                Grep(dir_entry.path(), pattern, visitor, options);
            } else {
                GetFile(dir_entry.path(), pattern, visitor, options);
            }
        }
    } else {
        GetFile(path, pattern, visitor, options);
    }
}