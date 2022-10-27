#pragma once

#include <type_traits>
#include <algorithm>
template <typename T, bool I, bool = std::is_empty_v<T> & !std::is_final_v<T>>
class CompressedEl {
public:
    CompressedEl() : value_(T()) {
    }
    CompressedEl(const T& value) : value_(value) {
    }
    CompressedEl(T&& val) : value_(std::move(val)) {
    }
    T& Get() {
        return value_;
    }
    const T& Get() const {
        return value_;
    }

private:
    T value_;
};

template <typename T, bool I>
class CompressedEl<T, I, true> : T {
public:
    CompressedEl() = default;

    CompressedEl(const T&) {
    }

    CompressedEl(T&&) {
    }

    T& Get() {
        return *this;
    }

    const T& Get() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : CompressedEl<F, 0>, CompressedEl<S, 1> {
public:
    CompressedPair() {
    }
    CompressedPair(const F& first, const S& second)
        : CompressedEl<F, 0>::CompressedEl(first), CompressedEl<S, 1>::CompressedEl(second) {
    }
    CompressedPair(const F& first, S&& second)
        : CompressedEl<F, 0>::CompressedEl(first),
          CompressedEl<S, 1>::CompressedEl(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second)
        : CompressedEl<F, 0>::CompressedEl(std::move(first)),
          CompressedEl<S, 1>::CompressedEl(second) {
    }
    CompressedPair(F&& first, S&& second)
        : CompressedEl<F, 0>::CompressedEl(std::move(first)),
          CompressedEl<S, 1>::CompressedEl(std::move(second)) {
    }
    F& GetFirst() {
        return CompressedEl<F, 0>::CompressedEl::Get();
    }

    const F& GetFirst() const {
        return CompressedEl<F, 0>::Get();
    }

    S& GetSecond() {
        return CompressedEl<S, 1>::Get();
    };

    const S& GetSecond() const {
        return CompressedEl<S, 1>::Get();
    };

private:
};