//
// Created by lukeh on 01/11/2019.
//
#include "FacebookAdKey.h"
#include "../internal_includes/DbUtils.h"

using namespace DbUtils;

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
