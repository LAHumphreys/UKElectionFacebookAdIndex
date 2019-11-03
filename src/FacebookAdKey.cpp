//
// Created by lukeh on 01/11/2019.
//
#include "FacebookAdKey.h"

namespace {
    constexpr bool IsWordChar(char c) {
            if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                return true;
            } else {
                return false;
            }
        }
    bool Search(const std::string toSearch, const std::string& key) {
        size_t pos = 0;
        bool found = false;
        while (!found && pos != std::string::npos) {
            pos = toSearch.find(key, pos);

            bool match = true;
            const size_t next = pos + key.size();

            if (pos == std::string::npos) {
                match = false;
            }
            if (match && next < toSearch.size()) {
                char nchar = toSearch[next];
                match = !IsWordChar(nchar);
            }

            if (match && pos > 0) {
                char pchar = toSearch[pos-1];
                match = !IsWordChar(pchar);
            }

            if (match) {
                found = true;
            } else if (pos != std::string::npos) {
                ++pos;
            }
        }

        return found;
    }
}


bool FacebookAdKey::HasKey(const StoredFacebookAd &item, const std::string &key) const {
    bool match = false;
    if (!item.IsNull()) {
        if (Search(item.CachedUppers().linkTitle, key)) {
            match = true;
        } else if (Search(item.CachedUppers().fundingEntity, key)) {
            match = true;
        } else if (Search(item.CachedUppers().pageName, key )) {
            match = true;
        } else if (Search(item.CachedUppers().body, key )) {
            match = true;
        } else if (Search(item.CachedUppers().linkDescription, key )) {
            match = true;
        } else if (Search(item.CachedUppers().linkCaption, key)) {
            match = true;
        }
    }
    return match;
}
