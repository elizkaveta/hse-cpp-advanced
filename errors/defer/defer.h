#pragma once
#include <iostream>
#include <algorithm>

template <typename Callback>
class Defer final {
public:
    Defer() = default;
    Defer(Callback callback) {
        ::new (GetCallbackBuffer()) Callback(std::move(callback));
    }
    void Invoke() {
        if (k_) {
            std::move(GetCallback())();
            k_ = 0;
        }
    }
    void Cancel() {
        k_ = 0;
        reinterpret_cast<Callback*>(GetCallbackBuffer())->~Callback();
    }
    template <class Y>
    Defer& operator=(const Y& c) {
        ::new (GetCallbackBuffer()) Y(std::move(c));
        return *this;
    }

    ~Defer() {
        if (k_) {
            std::move(GetCallback())();
        }
    }

private:
    bool k_ = 1;
    alignas(Callback) char callback_buffer_[sizeof(Callback)];

    void* GetCallbackBuffer() {
        return static_cast<void*>(callback_buffer_);
    }

    Callback& GetCallback() {
        return *reinterpret_cast<Callback*>(GetCallbackBuffer());
    }
};
