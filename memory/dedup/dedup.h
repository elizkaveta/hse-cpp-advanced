#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

using std::string;

std::vector<std::unique_ptr<string>> Duplicate(const std::vector<std::shared_ptr<string>>& items) {
    std::vector<std::unique_ptr<string>> unshared;
    for (size_t i = 0; i < items.size(); ++i) {
        unshared.emplace_back(std::make_unique<string>(*items[i]));
    }
    return unshared;
}

std::vector<std::shared_ptr<string>> DeDuplicate(
    const std::vector<std::unique_ptr<string>>& items) {
    std::vector<std::shared_ptr<string>> shared(items.size());
    std::unordered_map<string, size_t> index_ptr;
    for (size_t i = 0; i < items.size(); ++i) {
        if (index_ptr.contains(*items[i])) {
            shared[i] = shared[index_ptr[*items[i]]];
        } else {
            shared[i] = std::make_shared<string>(*items[i]);
            index_ptr[*items[i]] = i;
        }
    }
    return shared;
}
