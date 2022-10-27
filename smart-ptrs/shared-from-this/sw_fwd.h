#pragma once

#include <exception>
#include <array>
class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

struct ControlBlockBase {
public:
    size_t cnt_ = 0;
    size_t weak_ = 0;
    virtual ~ControlBlockBase() = default;

    virtual void Reset() = 0;
    virtual void PlusCounter() = 0;
    virtual void MinusCounter() = 0;
    virtual void PlusCounterWeak() = 0;
    virtual void MinusCounterWeak() = 0;
};

template <typename T>
class ControlBlockPointer : public ControlBlockBase {
public:
    ControlBlockPointer(T* ptr) : ptr_(ptr) {
        cnt_ = 1;
    }
    ~ControlBlockPointer() {
        if (ptr_ != nullptr && !cnt_) {
            delete ptr_;
            ptr_ = nullptr;
        }
        --cnt_;
    }

    void Reset() override {
        if (cnt_ == 1 && ptr_ != nullptr) {
            delete ptr_;
            ptr_ = nullptr;
        }
        if (cnt_) {
            MinusCounter();
        }
    }

    void PlusCounter() override {
        ++cnt_;
    }
    void MinusCounter() override {
        --cnt_;
    }

    void PlusCounterWeak() override {
        ++weak_;
    }
    void MinusCounterWeak() override {
        --weak_;
    }

    T* ptr_;
};

template <typename T>
class ControlBlockHolder : public ControlBlockBase {
public:
    template <typename... Args>
    ControlBlockHolder(Args&&... args) {
        new (&val_) T(std::forward<Args>(args)...);
        cnt_ = 1;
        ptr_ = reinterpret_cast<T*>(&val_);
    }
    ~ControlBlockHolder() {
        if (cnt_ == 0 && ptr_ != nullptr) {
            ptr_->~T();
        }
    }

    T* GetUk() {
        return reinterpret_cast<T*>(&val_);
    }

    void Reset() override {
        if (cnt_ == 0) {
            return;
        }
        if (cnt_ == 1 && ptr_ != nullptr) {
            ptr_->~T();
            ptr_ = nullptr;
        }
        MinusCounter();
        if (cnt_ == 0) {
            ptr_ = nullptr;
        }
    }

    void PlusCounter() override {
        ++cnt_;
    }
    void MinusCounter() override {
        --cnt_;
    }

    void PlusCounterWeak() override {
        ++weak_;
    }
    void MinusCounterWeak() override {
        --weak_;
    }

    T* ptr_;
    alignas(T) std::array<std::byte, sizeof(T)> val_;
};
