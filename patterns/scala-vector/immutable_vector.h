#pragma once

#include <vector>
#include <memory>
#include <iostream>

template <typename T>
class Bor {
public:
    template <typename M>
    class Block {
    public:
        Block(const M& value = M()) {
            value_ = value;
            blocks_.resize(size_);
        }
        void Set(size_t idx, std::shared_ptr<Block<M>> m) {
            blocks_[idx] = m;
        }

        std::shared_ptr<Block<M>> Get(size_t idx) {
            return blocks_[idx];
        }
        M& Value() {
            return value_;
        }

    private:
        M value_;
        const int size_ = 32;
        std::vector<std::shared_ptr<Block<M>>> blocks_;
    };
    Bor() {
    }
    Bor(std::shared_ptr<Block<T>> other_blocks) {
        blocks_ = other_blocks;
    }
    Bor(size_t count, const T& value = T()) {
        for (size_t i = 0; i < count; ++i) {
            blocks_ = Change(i, value);
            ++size_;
        }
    }
    template <typename Iterator>
    Bor(Iterator first, Iterator last) {
        while (first != last) {
            blocks_ = Change(size_, *first);
            ++size_;
            ++first;
        }
    }
    Bor(std::initializer_list<T> l) {
        for (auto& i : l) {
            blocks_ = Change(size_, i);
            ++size_;
        }
    }
    T& Get(size_t index) const {
        std::shared_ptr<Block<T>> ptr = blocks_;
        while (index) {
            ptr = ptr.get()->Get(index & mask_);
            index >>= log_;
        }
        return ptr.get()->Value();
    }
    void PreparePtr(std::shared_ptr<Block<T>>& ptr, const T& val) {
        ptr.reset(new Block<T>(ptr.use_count() ? *ptr.get() : val));
    }
    void ParePtr(size_t idx, std::shared_ptr<Block<T>>& ptr, const T& val) {
        while (idx) {
            Nxt(idx, ptr);
        }
        ptr.get()->Value() = val;
    }

    std::shared_ptr<Block<T>> Change(size_t idx, const T& val) {
        std::shared_ptr<Block<T>> ptr(blocks_);
        PreparePtr(ptr, val);
        std::shared_ptr<Block<T>> new_ptr(ptr);
        ParePtr(idx, ptr, val);
        return new_ptr;
    }

    void Nxt(size_t& idx, std::shared_ptr<Block<T>>& ptr) {
        size_t block = (mask_ & idx);
        ptr.get()->Set(block,
                       std::shared_ptr<Block<T>>(ptr.get()->Get(block).use_count()
                                                     ? new Block<T>(*ptr.get()->Get(block).get())
                                                     : new Block<T>()));
        idx >>= log_;
        ptr = ptr.get()->Get(block);
    }

private:
    std::shared_ptr<Block<T>> blocks_;
    size_t size_ = 0;
    int mask_ = 31;
    int log_ = 5;
    template <typename X>
    friend class ImmutableVector;
};

template <class T>
class ImmutableVector {
private:
public:
    ImmutableVector() {
    }

    explicit ImmutableVector(size_t count, const T& value = T()) {
        vec_ = Bor<T>(count, value);
    }

    template <typename Iterator>
    ImmutableVector(Iterator first, Iterator last) {
        vec_ = Bor<T>(first, last);
    }

    ImmutableVector(std::initializer_list<T> l) {
        vec_ = Bor<T>(l);
    }
    ImmutableVector(const Bor<T>& other, int sz) {
        vec_ = other;
        vec_.size_ = sz + other.size_;
    }

    ImmutableVector Set(size_t index, const T& value) {
        ImmutableVector<T> result(Bor<T>(vec_.Change(index, value)), 0);
        result.vec_.size_ = vec_.size_;
        return result;
    }

    const T& Get(size_t index) const {
        return vec_.Get(index);
    }

    ImmutableVector PushBack(const T& value) {
        ImmutableVector<T> result(Bor<T>(vec_.Change(vec_.size_, value)), 0);
        result.vec_.size_ = vec_.size_ + 1;
        return result;
    }

    ImmutableVector PopBack() {
        if (!vec_.size_) {
            return *this;
        }
        return ImmutableVector<T>(vec_, -1);
    }

    size_t Size() const {
        return vec_.size_;
    }

private:
    Bor<T> vec_;
};