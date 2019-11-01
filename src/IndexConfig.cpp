//
// Created by lukeh on 01/11/2019.
//
#include <IndexConfig.h>
#include <locale>

namespace {
    void ToUpper(std::string& target) {
        thread_local auto& f = std::use_facet<std::ctype<char>>(std::locale());
        f.toupper(&target[0], &target[0] + target.size());
    }
}

IndexConfig::IndexConfig(std::vector<Item> items) {
    (*this) = std::move(items);
}

IndexConfig &IndexConfig::operator=(std::vector<Item> source) {
    this->items = source;
    for (Item& i: items) {
        for (std::string& key: i.keys) {
            ToUpper(key);
        }
    }

    return *this;
}
