#pragma once

#include <unistd.h>
#include "sw_fwd.h"  // Forward declaration
#include <type_traits>
#include <memory>
#include <utility>

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_ptr_);
    }
    SharedPtr<const T> SharedFromThis() const {
        return weak_ptr_;
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_ptr_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return weak_ptr_;
    }

private:
    WeakPtr<T> weak_ptr_;
    template <typename P>
    friend class SharedPtr;
};

template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    SharedPtr() {
        block_ = nullptr;
        ptr_ = nullptr;
    }
    SharedPtr(std::nullptr_t) {
        ptr_ = nullptr;
        block_ = nullptr;
    }
    explicit SharedPtr(T* ptr) {
        ptr_ = ptr;
        block_ = new ControlBlockPointer<T>(ptr);
        EnableShared(ptr);
    }

    SharedPtr(T* ptr, ControlBlockBase* block) {
        block_ = block;
        ptr_ = ptr;
        EnableShared(ptr);
    }

    template <typename X>
    explicit SharedPtr(X* ptr) {
        ptr_ = ptr;
        block_ = new ControlBlockPointer(ptr);
        EnableShared(ptr);
    }

    SharedPtr(const SharedPtr& other) {
        ptr_ = other.Get();
        block_ = other.block_;
        PlusBlock();
    }
    SharedPtr(SharedPtr&& other) {
        ptr_ = other.Get();
        block_ = other.block_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    template <typename X>
    SharedPtr(const SharedPtr<X>& other) {
        ptr_ = other.Get();
        block_ = other.block_;
        PlusBlock();
    }
    template <typename X>
    SharedPtr(SharedPtr<X>&& other) {
        ptr_ = other.Get();
        block_ = other.block_;

        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        ptr_ = ptr;
        block_ = other.block_;
        PlusBlock();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        ptr_ = other.ptr_;
        if (other.block_ == nullptr) {
            throw BadWeakPtr();
        } else {
            if (other.block_->cnt_ == 0) {
                throw BadWeakPtr();
            }
            block_ = other.block_;
            block_->cnt_++;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <typename X>
    SharedPtr& operator=(SharedPtr<X>&& other) {
        ResetAndDelete();
        ptr_ = other.Get();
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    template <typename X>
    SharedPtr& operator=(const SharedPtr<X>& other) {
        ResetAndDelete();
        ptr_ = other.Get();
        block_ = other.block_;
        PlusBlock();
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (&other == this) {
            return *this;
        }
        ResetAndDelete();
        ptr_ = other.Get();
        block_ = other.block_;
        PlusBlock();
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (&other == this) {
            return *this;
        }
        ResetAndDelete();
        ptr_ = other.Get();
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    ~SharedPtr() {
        ResetAndDelete();
    }

    void Reset() {
        ResetAndDelete();
        block_ = nullptr;
        ptr_ = nullptr;
    }
    template <typename X>
    void Reset(X* ptr) {
        ResetAndDelete();
        block_ = new ControlBlockPointer<X>(ptr);
        ptr_ = ptr;
    }
    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->cnt_;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }
    template <typename X>
    std::enable_if_t<std::is_base_of_v<EnableSharedFromThis<X>, T>> EnableShared(
        EnableSharedFromThis<X>* ptr) {
        ptr->weak_ptr_ = WeakPtr<T>(*this);
    }
    void EnableShared(...) {
    }

private:
    T* ptr_ = nullptr;
    ControlBlockBase* block_ = nullptr;

    void ResetAndDelete() {
        ResetBlock();
        MinusBlock();
    }
    void MinusBlock() {
        if (block_ && block_->cnt_ + block_->weak_ == 0 && block_ != nullptr) {
            delete block_;
            ptr_ = nullptr;
        }
    }

    void PlusBlock() {
        if (block_) {
            block_->PlusCounter();
        } else {
            return;
        }
    }

    void ResetBlock() {
        if (block_) {
            block_->Reset();
        }
    }
    template <typename X>
    friend class WeakPtr;
    template <typename X>
    friend class SharedPtr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    ControlBlockHolder<T>* block_chain = new ControlBlockHolder<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block_chain->GetUk(), block_chain);
}
