//
// Created by lukeh on 01/11/2019.
//
#include <FacebookAd.h>
#include <FacebookAdStore.h>
#include <locale>

namespace {
    constexpr StoredFacebookAd::KeyType CalculateKey(const FacebookAd& ad) {
        return ad.id;
    }

    std::string ToUpper(const std::string& target) {
        std::string upper(target);
        thread_local auto& f = std::use_facet<std::ctype<char>>(std::locale());
        f.toupper(&upper[0], &upper[0] + upper.size());

        return upper;
    }
}

#include <FacebookAdStore.h>
StoredFacebookAd::StoredFacebookAd(std::shared_ptr<FacebookAd> ad)
    : item(std::move(ad))
{
    key = CalculateKey(*item);
    upperCase.linkTitle = ToUpper(item->linkTitle);
    upperCase.body = ToUpper(item->body);
    upperCase.linkDescription = ToUpper(item->linkDescription);
    upperCase.linkCaption = ToUpper(item->linkCaption);
    upperCase.fundingEntity = ToUpper(item->fundingEntity);
    upperCase.pageName = ToUpper(item->pageName);
}


void StoredFacebookAd::PatchStoredValues(FacebookAd &&ad) {
    if (!IsNull()) {
        *item = std::move(ad);
        auto newKey = CalculateKey(*item);
        if (newKey != key) {
            throw KeyChangeError{};
        }
    }
}

const StoredFacebookAd &FacebookAdStore::Get(const StoredFacebookAd::KeyType& key) const {
    auto it = ads.find(key);

    if (it != ads.end()) {
        return *it->second;
    } else {
        return this->NullAdd;
    }
}

const StoredFacebookAd &FacebookAdStore::Store(std::unique_ptr<FacebookAd> ad) {
    StoredFacebookAd* result = &NullAdd;
    auto storedAd = std::make_unique<StoredFacebookAd>(std::move(ad));
    auto it = ads.find(storedAd->Key());
    if (it != ads.end()) {
        it->second->PatchStoredValues(std::move(*(storedAd->item)));
    } else {
        result = ads.try_emplace(storedAd->Key(), std::move(storedAd)).first->second.get();
    }
    return *result;
}

