#pragma once

#include <string>

// Should not allow reuse and yell under sanitizers.
// Fix the interface and implementation.
// AwesomeCallback should add "awesomeness".

class OneTimeCallback {
public:
    virtual ~OneTimeCallback() {
    }
    virtual std::string operator()() const&& = 0;
};

// Implement ctor, operator(), maybe something else...
class AwesomeCallback : public OneTimeCallback {
public:
    AwesomeCallback(const std::string& mew) {
        mew_ = mew + plus_mew_;
    }
    std::string operator()() const&& override {
        std::string new_mew = mew_;
        delete this;
        return new_mew;
    }

private:
    std::string mew_;
    std::string plus_mew_ = "awesomeness";
};
