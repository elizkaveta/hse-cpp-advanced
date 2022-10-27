#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct State {
    int ref_count = 1;  // сколько векторов делят этот State между собой.
    std::vector<std::string> s;
    State(std::vector<std::string> ss) : s(ss) {
    }
    State() {
    }
};

class COWVector {
public:
    COWVector() {
        state_ = new State();
    }
    ~COWVector() {
        state_->ref_count--;
        if (!state_->ref_count) {
            delete state_;
        }
    }

    COWVector(const COWVector& other) {
        state_ = other.state_;
        ++state_->ref_count;
    };
    COWVector& operator=(const COWVector& other) {
        if (this == &other) {
            return *this;
        } else {
            --state_->ref_count;
        }
        if (state_->ref_count == 0) {
            delete state_;
        }
        state_ = other.state_;
        ++state_->ref_count;
        return *this;
    }

    size_t Size() const {
        return state_->s.size();
    }

    void Resize(size_t size) {
        if (state_->ref_count > 1) {
            state_->ref_count--;
            if (state_->ref_count == 0) {
                delete state_;
            }
            state_ = new State(*state_);
            state_->ref_count = 1;
            state_->s.resize(size);
        } else {
            state_->s.resize(size);
        }
    }
    const std::string& Get(size_t at) {
        return state_->s[at];
    }
    const std::string& Back() {
        return state_->s.back();
    }

    void PushBack(const std::string& value) {
        if (state_->ref_count > 1) {
            state_->ref_count--;
            if (state_->ref_count == 0) {
                delete state_;
            }
            state_ = new State(*state_);
            state_->ref_count = 1;
            state_->s.push_back(value);
        } else {
            state_->s.push_back(value);
        }
    }

    void Set(size_t at, const std::string& value) {
        if (state_->ref_count > 1) {
            state_->ref_count--;
            state_ = new State(*state_);
            state_->ref_count = 1;
            state_->s[at] = value;
        } else {
            state_->s[at] = value;
        }
    }

private:
    State* state_;
};
