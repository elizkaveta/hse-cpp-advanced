#pragma once

#include <initializer_list>
#include <algorithm>
#include <deque>
#include <iostream>

class Deque {
public:
    Deque() = default;
    Deque(const Deque& rhs) {
        for (size_t i = 0; i < rhs.Size(); i++) {
            PushBack(rhs[i]);
        }
    }
    Deque(Deque&& rhs) {
        for (size_t i = 0; i < rhs.Size(); i++) {
            PushBack(rhs[i]);
        }
    };
    explicit Deque(size_t size) {
        for (size_t i = 0; i < size; ++i) {
            PushBack(0);
        }
    }

    Deque(std::initializer_list<int> list) {
        for (auto i : list) {
            PushBack(i);
        }
    }
    ~Deque() {
        Clear();
        delete[] data_;
    }
    Deque& operator=(Deque rhs) {
        if (rhs.Size() == 0 && Size() == 0) {
            return *this;
        }
        if (data_[beg_] == rhs.data_[rhs.beg_]) {
            return *this;
        }
        Clear();
        Swap(rhs);
        return *this;
    }

    void Swap(Deque& rhs) {
        if (rhs.Size() == 0 && Size() == 0) {
            return;
        }
        if (data_[beg_] == rhs.data_[rhs.beg_]) {
            return;
        }
        std::swap(data_, rhs.data_);
        std::swap(beg_, rhs.beg_);
        std::swap(end_, rhs.end_);
        std::swap(size_, rhs.size_);
        std::swap(capacity_, rhs.capacity_);
    }
    size_t size_ = 0;
    size_t beg_ = 0;
    size_t end_ = 0;
    void PushBack(int value) {
        if (size_ == 0) {
            beg_ = 0;
            end_ = 0;
            data_[0] = new Block();
            size_++;
            data_[0]->PushBack(value);
            return;
        }
        if (data_[end_]->end_ == kSizeOfBlock - 1) {
            if (GetNext(end_) == beg_) {
                Rebase();
            }
            end_++;
            data_[end_] = new Block();
        }
        data_[end_]->PushBack(value);
        ++size_;
    }

    void PopBack() {
        data_[end_]->PopBack();
        if (data_[end_]->Size() == 0) {
            delete data_[end_];
            end_ = (end_ + capacity_ - 1) % capacity_;
        }
        --size_;
    }

    void PushFront(int value) {
        if (size_ == 0) {
            beg_ = 0;
            end_ = 0;
            data_[0] = new Block();
            data_[0]->PushBack(value);
            size_++;
            return;
        }
        if (data_[beg_]->beg_ == 0) {
            if (GetPrev(beg_) == end_) {
                Rebase();
            }
            if (beg_ == 0) {
                beg_ = capacity_;
            }
            --beg_;
            data_[beg_] = new Block();
        }

        data_[beg_]->PushFront(value);
        ++size_;
    }

    void PopFront() {
        data_[beg_]->PopFront();
        if (data_[beg_]->Size() == 0) {
            delete data_[beg_];
            beg_ = (beg_ + 1) % capacity_;
        }
        --size_;
    }

    int& operator[](size_t ind) {
        size_t sz = data_[beg_]->Size();
        if (sz > ind) {
            return data_[beg_]->Get(ind);
        }
        return data_[(beg_ + 1 + static_cast<int>(ind - sz) / kSizeOfBlock) % capacity_]->Get(
            (ind - sz) % kSizeOfBlock);
    }

    int operator[](size_t ind) const {
        size_t sz = data_[beg_]->Size();
        if (sz > ind) {
            return data_[beg_]->Get(ind);
        }
        return data_[(beg_ + 1 + static_cast<int>(ind - sz) / kSizeOfBlock) % capacity_]->Get(
            (ind - sz) % kSizeOfBlock);
    }

    size_t Size() const {
        return size_;
    }

    void Clear() {
        size_t i = 0;
        while (size_) {
            size_ -= data_[beg_]->Size();
            delete data_[beg_];
            ++i;
            ++beg_;
            if (beg_ == capacity_) {
                beg_ = 0;
            }
        }
        beg_ = 0;
        end_ = 0;
    }
    size_t capacity_ = 1;

private:
    static const size_t kSizeOfBlock = 128;
    class Block {
    public:
        Block() {
            beg_ = 0;
            end_ = 0;
            size_ = 0;
        };
        int Get(size_t i) const {
            return vec[beg_ + i];
        }
        int& Get(size_t i) {
            return vec[beg_ + i];
        }
        void PopBack() {
            --end_;
            --size_;
        }
        void PopFront() {
            ++beg_;
            --size_;
        }
        void PushBack(int i) {
            if (size_ == 0) {
                beg_ = 0;
                end_ = 0;
                vec[end_] = i;
                ++size_;
                return;
            }
            vec[end_ + 1] = i;
            ++end_;
            ++size_;
        }
        void PushFront(int i) {
            if (size_ == 0) {
                beg_ = kSizeOfBlock;
                end_ = kSizeOfBlock - 1;
            }
            vec[beg_ - 1] = i;
            --beg_;
            ++size_;
        }
        int vec[kSizeOfBlock];
        size_t beg_ = 0;
        size_t end_ = 0;
        size_t size_ = 0;
        size_t Size() const {
            return size_;
        }
    };
    size_t GetNext(size_t i) const {
        if (i + 1 == capacity_) {
            return 0;
        }
        return i + 1;
    }
    size_t GetPrev(size_t i) const {
        if (i == 0) {
            return capacity_ - 1;
        }
        return i - 1;
    }
    Block** data_ = new Block*[1];

    void Rebase() {

        capacity_ *= 2;

        Block** a = new Block*[capacity_];
        size_t sz = 0;
        if (size_ == 0) {
            a[0] = std::move(data_[beg_]);
        } else {
            for (size_t i = 0; sz < size_; ++i) {
                a[i] = std::move(data_[(i + beg_) % (capacity_ / 2)]);
                sz += a[i]->Size();
                end_ = i;
            }
        }
        std::swap(data_, a);
        delete[] a;
        beg_ = 0;
    }
};
