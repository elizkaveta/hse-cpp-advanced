#pragma once
#include <vector>

template <class RandomIt, class F, class P>
void TransformIf(RandomIt begin, RandomIt end, P p, F f) {
    std::vector<typeof(*begin)> a;
    auto it = begin;
    while (it != end) {
        try {
            a.push_back(*it);

        } catch (...) {
            a.push_back(*it);
        }
        bool x = 0;
        try {
            x = p(a.back());
        } catch (...) {
            for (auto& i : a) {
                *begin = i;
                begin++;
            }
            throw;
        }
        try {
            if (x) {
                f(*it);
            }
        } catch (...) {
            for (auto& i : a) {
                *begin = i;
                begin++;
            }
            throw;
        }
        it++;
    }
}