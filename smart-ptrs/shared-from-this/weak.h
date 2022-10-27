#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"
#include <iostream>

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() = default;

    WeakPtr(const WeakPtr& other) {
        ptr_ = static_cast<T*>(other.ptr_);
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->PlusCounterWeak();
        }
    }
    template <typename S>
    WeakPtr(const WeakPtr<S>& other) {
        ptr_ = static_cast<T*>(other.ptr_);
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->PlusCounterWeak();
        }
    }

    WeakPtr(WeakPtr&& other) {
        ptr_ = std::move(other.ptr_);
        other.ptr_ = nullptr;
        block_ = std::move(other.block_);
        other.block_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        block_->PlusCounterWeak();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (other.block_ != nullptr) {
            block_->PlusCounterWeak();
        }
        return *this;
    }
    template <typename U>
    WeakPtr& operator=(const WeakPtr<U>& other) {
        Reset();
        ptr_ = static_cast<T*>(other.ptr_);
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->PlusCounterWeak();
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (this == &other) {
            return *this;
        }
        Reset();
        ptr_ = std::move(other.ptr_);
        block_ = std::move(other.block_);
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ != nullptr) {
            --block_->weak_;
            if (!block_->cnt_ && !block_->weak_) {
                delete block_;
            }
            block_ = nullptr;
            ptr_ = nullptr;
        }
    }

    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!block_) {
            return 0;
        } else {
            return block_->cnt_;
        }
    }

    bool Expired() const {
        return UseCount() == 0;
    }

    SharedPtr<T> Lock() const {
        if (block_ == nullptr) {
            return SharedPtr<T>(nullptr, new ControlBlockPointer<T>(nullptr));
        }
        if (block_->cnt_ == 0) {
            ptr_ = nullptr;
        }
        block_->PlusCounter();
        return SharedPtr(ptr_, block_);
    }

private:
    ControlBlockBase* block_ = nullptr;
    mutable T* ptr_ = nullptr;
    template <typename P>
    friend class SharedPtr;
    template <typename P>
    friend class WeakPtr;
};
