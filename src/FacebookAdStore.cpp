//
// Created by lukeh on 01/11/2019.
//
#include <FacebookAd.h>
#include <FacebookAdStore.h>
#include <FacebookParser.h>
#include <locale>
#include "../internal_includes/StoredItemJSON.h"
#include "../internal_includes/StoreJSON.h"

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

    void DeSerialize(const std::string& ser,
                     std::unique_ptr<FacebookAd>& ad,
                     StoredFacebookAd::UpperStrings& uppers)
    {
        using namespace StoredItem;
        thread_local FacebookAdParser parser;
        thread_local JSON encoder;
        encoder.Clear();

        std::string error;
        if (encoder.Parse(ser.c_str(), error)) {
            parser.DeSerialize(std::move(encoder.Get<item>()), ad);
            uppers.fundingEntity = std::move(encoder.Get<upperFundingEntity>());
            uppers.pageName = std::move(encoder.Get<upperPageName>());
            uppers.body = std::move(encoder.Get<upperBody>());
            uppers.linkDescription = std::move(encoder.Get<upperDescription>());
            uppers.linkTitle = std::move(encoder.Get<upperTitle>());
            uppers.linkCaption = std::move(encoder.Get<upperCaption>());
        }
    }

    std::string Serialize(const StoredFacebookAd& ad) {
        using namespace StoredItem;
        thread_local FacebookAdParser parser;
        thread_local JSON encoder;
        encoder.Clear();
        encoder.Get<item>() = parser.Serialize(ad.ItemRef());
        encoder.Get<upperTitle>() = ad.CachedUppers().linkTitle;
        encoder.Get<upperBody>() = ad.CachedUppers().body;
        encoder.Get<upperDescription>() = ad.CachedUppers().linkDescription;
        encoder.Get<upperCaption>() = ad.CachedUppers().linkCaption;
        encoder.Get<upperFundingEntity>() = ad.CachedUppers().fundingEntity;
        encoder.Get<upperPageName>() = ad.CachedUppers().pageName;

        return encoder.GetJSONString();
    }

}

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

StoredFacebookAd::StoredFacebookAd(const StoredFacebookAd::Serialization &ad)
{
    std::unique_ptr<FacebookAd> uad;
    DeSerialize(ad.json, uad, upperCase);
    item = std::move(uad);
    if (item.get()) {
        key = CalculateKey(*item);
    }
}

StoredFacebookAd::Serialization StoredFacebookAd::Serialize() const {
    return {::Serialize(*this)};
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

FacebookAdStore::Serialization FacebookAdStore::Serialize() const {
    thread_local Store::JSON encoder;
    encoder.Clear();
    for (auto it = ads.begin(); it != ads.end(); ++it) {
        auto& jitem = encoder.Get<Store::store>().emplace_back();

        jitem->Get<Store::key>() = it->first;
        jitem->Get<Store::ad>() = std::move(it->second->Serialize().json);
    }

    return {encoder.GetJSONString()};

}

FacebookAdStore::FacebookAdStore(FacebookAdStore::Serialization data) {
    thread_local Store::JSON decoder;
    decoder.Clear();
    std::string error;

    if (decoder.Parse(data.data.c_str(), error)) {
        for (auto& jitem: decoder.Get<Store::store>()) {
            StoredFacebookAd::Serialization adData;
            adData.json = std::move(jitem->Get<Store::ad>());
            ads[jitem->Get<Store::key>()] =
                    std::make_unique<StoredFacebookAd>(adData);
        }
    } else {
        throw InvalidSerialization{};
    }

}


