#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <memory>

template <typename P>
struct Slug {
    constexpr Slug() noexcept = default;
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, P*>::value>::type>
    Slug(const Slug<U>&) noexcept {
    }

    void operator()(P* ptr) const {
        delete ptr;
    }
};

template <typename V>
struct Slug<V[]> {
public:
    constexpr Slug() noexcept = default;

    template <typename O, typename = typename std::enable_if<
                              std::is_convertible<O (*)[], V (*)[]>::value>::type>
    Slug(const Slug<O[]>&) noexcept {
    }

    template <typename U>
    typename std::enable_if<std::is_convertible<U (*)[], V (*)[]>::value>::type operator()(
        U* ptr) const {
        delete[] ptr;
    }
};
template <typename T, typename Deleter = Slug<T> >
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    template <typename X = T>
    explicit UniquePtr(X* ptr = nullptr) : val_(static_cast<T*>(ptr), Deleter()) {
    }

    template <typename X, typename Del>
    UniquePtr(X ptr, Del&& deleter) : val_(static_cast<T*>(ptr), std::forward<Del>(deleter)) {
    }

    template <typename X, typename Del>
    UniquePtr(UniquePtr<X, Del>&& other) noexcept {
        val_.GetSecond() = std::move(other.val_.GetSecond());
        val_.GetFirst() = static_cast<T*>(other.val_.GetFirst());
        other.val_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Reset(other.Release());
        val_.GetSecond() = std::forward<Deleter>(other.val_.GetSecond());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        val_.GetSecond()(val_.GetFirst());
        val_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* val = val_.GetFirst();
        val_.GetFirst() = nullptr;
        return val;
    }

    void Reset(T* ptr = nullptr) {
        if (ptr == val_.GetFirst()) {
            return;
        }
        std::swap(ptr, val_.GetFirst());
        val_.GetSecond()(ptr);
    }

    void Swap(UniquePtr& other) {
        std::swap(other.val_.GetFirst(), val_.GetFirst());
        std::swap(other.val_.GetSecond(), val_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return val_.GetFirst();
    }

    Deleter& GetDeleter() {
        return val_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return val_.GetSecond();
    }

    explicit operator bool() const {
        return val_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *val_.GetFirst();
    }

    T* operator->() const {
        return val_.GetFirst();
    }

    template <typename M, typename Deleter2>
    friend class UniquePtr;

private:
    CompressedPair<T*, Deleter> val_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    template <typename M = T>
    explicit UniquePtr(M* ptr = nullptr) : val_(static_cast<T*>(ptr), Deleter()) {
    }

    template <typename Pointer, typename DeleterType>
    UniquePtr(Pointer ptr, const DeleterType& deleter)
        : val_(static_cast<T*>(ptr), std::forward<DeleterType>(deleter)) {
    }

    template <typename M, typename DeleterType>
    UniquePtr(UniquePtr<M, DeleterType>&& other) noexcept {
        val_.GetFirst() = static_cast<T*>(other.val_.GetFirst());
        val_.GetSecond() = std::move(other.val_.GetSecond());
        other.val_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        val_.GetSecond()(val_.GetFirst());
        val_.GetFirst() = static_cast<T*>(other.val_.GetFirst());
        val_.GetSecond() = std::move(other.val_.GetSecond());
        other.val_.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        val_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        val_.GetSecond()(val_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* arr = val_.GetFirst();
        val_.GetFirst() = nullptr;
        return arr;
    }

    void Reset(T* ptr = nullptr) {
        if (ptr != val_.GetFirst()) {
            std::swap(val_.GetFirst(), ptr);
            val_.GetSecond()(ptr);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(other.val_.GetFirst(), val_.GetFirst());
        std::swap(other.val_.GetSecond(), val_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return val_.GetFirst();
    }

    Deleter& GetDeleter() {
        return val_.GetSecond();
    }

    const Deleter& GetDeleter() const {
        return val_.GetSecond();
    }

    explicit operator bool() const {
        return val_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Muttiple-object dereference operators

    T& operator*() const {
        return *val_.GetFirst();
    }

    T* operator->() {
        return val_.GetFirst();
    }

    T& operator[](size_t index) {
        return val_.GetFirst()[index];
    }

    const T& operator[](size_t index) const {
        return val_.GetFirst()[index];
    }

private:
    CompressedPair<T*, Deleter> val_;
};