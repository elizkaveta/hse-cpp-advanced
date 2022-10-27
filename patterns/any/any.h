#pragma once

#include <memory>

class Any {
public:
    Any() {
    }

    template <class T>
    Any(const T& value) : ptr_(new Inner(value)) {
    }

    template <class T>
    Any& operator=(const T& value) {
        ptr_.reset(new Inner<T>(value));
        return *this;
    }

    Any(const Any& rhs) {
        if (rhs.ptr_ != nullptr) {
            ptr_ = rhs.ptr_->GetNewPtr();
        }
    }
    Any& operator=(const Any& rhs) {
        Any new_any(rhs);
        Swap(new_any);
        return *this;
    }

    ~Any() = default;

    bool Empty() const {
        return ptr_ == nullptr;
    }

    void Clear() {
        ptr_ = nullptr;
    }
    void Swap(Any& rhs) {
        std::swap(ptr_, rhs.ptr_);
    }

    template <class T>
    const T& GetValue() const {
        return dynamic_cast<const Inner<T>&>(*ptr_).value;
    }

private:
    struct Base {
        using Ptr = std::unique_ptr<Base>;
        virtual ~Base() = default;
        virtual std::unique_ptr<Base> GetNewPtr() = 0;
    };
    template <typename T>
    struct Inner : Base {
        T value;
        Inner(T new_val) : value(new_val) {
        }
        Ptr GetNewPtr() {
            return std::make_unique<Inner<T>>(value);
        }
    };
    Base::Ptr ptr_ = nullptr;
};
