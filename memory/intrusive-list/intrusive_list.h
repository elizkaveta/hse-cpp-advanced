#pragma once

#include <algorithm>
#include <iostream>

class ListHook {
public:
    ListHook() {
        left_ = this;
        right_ = this;
        is_linked_ = 0;
    }
    bool IsLinked() const {
        return is_linked_;
    }

    void Unlink() {
        left_->right_ = right_;
        right_->left_ = left_;
        left_ = nullptr;
        right_ = nullptr;
        is_linked_ = false;
    }

    // Must unlink element from list
    ~ListHook() {
        if (left_) {
            left_->right_ = right_;
        }
        if (right_) {
            right_->left_ = left_;
        }
        left_ = nullptr;
        right_ = nullptr;
        is_linked_ = false;
    }

    ListHook(const ListHook& a) {
        left_ = a.left_;
        right_ = a.right_;
        is_linked_ = a.is_linked_;
    }

private:
    template <class T>
    friend class List;
    ListHook* left_ = nullptr;
    ListHook* right_ = nullptr;
    // that helper function might be useful
    void LinkBefore(ListHook* other) {
        left_ = other->left_;
        left_->right_ = this;
        right_ = other;
        other->left_ = this;
        is_linked_ = true;
    }
    bool is_linked_ = false;
};

template <typename T>
class List {
public:
    class Iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
    public:
        Iterator(T* val) : val_(val){};
        Iterator& operator++() {
            val_ = static_cast<T*>(val_->right_);
            return *this;
        }
        Iterator operator++(int) {
            val_ = static_cast<T*>(val_->right_);
            return *this;
        }

        T& operator*() const {
            return *val_;
        }
        T* operator->() const {
            return val_;
        }

        bool operator==(const Iterator& rhs) const {
            return val_ == rhs.val_;
        }
        bool operator!=(const Iterator& rhs) const {
            return val_ != rhs.val_;
        }

        T* val_;
    };

    List() = default;
    List(const List& a) = default;
    List(List&& other) {
        dummy_ = other.dummy_;
        if (dummy_.right_) {
            dummy_.right_->left_ = &dummy_;
        }
        if (dummy_.left_) {
            dummy_.left_->right_ = &dummy_;
        }
        other.dummy_.left_ = &other.dummy_;
        other.dummy_.right_ = &other.dummy_;
    };

    // must unlink all elements from list
    ~List() {
        while (dummy_.right_ != &dummy_) {
            dummy_.right_->Unlink();
        }
    }

    List& operator=(const List& a) = default;
    List& operator=(List&& other) {
        dummy_ = other.dummy_;
        if (dummy_.right_) {
            dummy_.right_->left_ = &dummy_;
        }
        if (dummy_.left_) {
            dummy_.left_->right_ = &dummy_;
        }
        other.dummy_.left_ = &other.dummy_;
        other.dummy_.right_ = &other.dummy_;
        return *this;
    }

    bool IsEmpty() const {
        return dummy_.left_ == &dummy_;
    }
    // that method is allowed to be O(n)
    size_t Size() const {
        size_t sz = 0;
        auto i = dummy_.right_;
        while (i != &dummy_) {
            i = i->right_;
            sz++;
        }
        return sz;
    }

    // note that IntrusiveList doesn't own elements,
    // and never copies or moves T
    void PushBack(T* elem) {
        elem->LinkBefore(&dummy_);
    }
    void PushFront(T* elem) {
        elem->LinkBefore(dummy_.right_);
    }

    T& Front() {
        return *static_cast<T*>(dummy_.right_);
    }
    const T& Front() const {
        return *static_cast<T*>(dummy_.right_);
    }

    T& Back() {
        return *static_cast<T*>(dummy_.left_);
    }

    const T& Back() const {
        return *static_cast<T*>(dummy_.left_);
    }

    void PopBack() {
        dummy_.left_->Unlink();
    }
    void PopFront() {
        dummy_.right_->Unlink();
    }

    Iterator Begin() {
        return Iterator(static_cast<T*>(dummy_.right_));
    }
    Iterator End() {
        return Iterator(static_cast<T*>(&dummy_));
    }

    // complexity of this function must be O(1)
    Iterator IteratorTo(T* element) {
        return Iterator(element);
    }

private:
    ListHook dummy_ = ListHook();
};

template <typename T>
typename List<T>::Iterator begin(List<T>& list) {  // NOLINT
    return list.Begin();
}

template <typename T>
typename List<T>::Iterator end(List<T>& list) {  // NOLINT
    return list.End();
}
