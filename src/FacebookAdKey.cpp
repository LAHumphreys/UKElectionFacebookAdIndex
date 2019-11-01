//
// Created by lukeh on 01/11/2019.
//
#include "FacebookAdKey.h"


bool FacebookAdKey::HasKey(const StoredFacebookAd &item, const std::string &key) const {
    bool match = false;
    if (!item.IsNull()) {
        if (item.CachedUppers().linkTitle.find(key) != std::string::npos) {
            match = true;
        } else if (item.CachedUppers().fundingEntity.find(key) != std::string::npos) {
            match = true;
        } else if (item.CachedUppers().pageName.find(key) != std::string::npos) {
            match = true;
        } else if (item.CachedUppers().body.find(key) != std::string::npos) {
            match = true;
        } else if (item.CachedUppers().linkDescription.find(key) != std::string::npos) {
            match = true;
        } else if (item.CachedUppers().linkCaption.find(key) != std::string::npos) {
            match = true;
        }
    }
    return match;
}
