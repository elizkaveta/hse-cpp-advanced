#include "lru_cache.h"
#include <iostream>

LruCache::LruCache(size_t max_size) {
    max_size_ = max_size;
}

void LruCache::Set(const std::string& key, const std::string& value) {
    if (data_.contains(key)) {
        AddValueToEnd(key);
        data_[key]->first = value;
        return;
    }
    list_.push_front({value, key});
    data_[key] = list_.begin();
    if (list_.size() > max_size_) {
        data_.erase(list_.back().second);
        list_.pop_back();
    }
}

bool LruCache::Get(const std::string& key, std::string* value) {
    if (data_.contains(key)) {
        *value = data_[key]->first;
        AddValueToEnd(key);
        return true;
    }
    return false;
}
void LruCache::AddValueToEnd(const std::string& key) {
    list_.push_front({data_[key]->first, key});
    list_.erase(data_[key]);
    data_[key] = list_.begin();
}
