//
// Created by lukeh on 01/11/2019.
//

#ifndef ELECTIONDATAANAL_FACEBOOKADSTORE_H
#define ELECTIONDATAANAL_FACEBOOKADSTORE_H
#include <memory>
#include <map>
#include <FacebookAd.h>

class StoredFacebookAd {
public:
    using KeyType = std::string;
    struct KeyRef {
        KeyRef(const KeyType& key): key(key) {}
        bool operator<(const KeyRef& rhs) const { return key < rhs.key; }
        const KeyType& key;
    };

    struct UpperStrings {
        std::string linkTitle;
        std::string body;
        std::string linkDescription;
        std::string linkCaption;
        std::string fundingEntity;
        std::string pageName;
    };

    StoredFacebookAd() = default;
    explicit StoredFacebookAd(std::shared_ptr<FacebookAd> ad);

    [[nodiscard]] const FacebookAd& ItemRef() const { return *item;}
    [[nodiscard]] std::shared_ptr<const FacebookAd> NewSharedRef() const { return item;}
    [[nodiscard]] const KeyType& Key() const { return key;}

    [[nodiscard]] const UpperStrings& CachedUppers() const { return upperCase;}

    [[nodiscard]] bool IsNull() const { return (item == nullptr); }

    void PatchStoredValues(FacebookAd&& patchFrom);

    struct KeyChangeError: std::exception {
        [[nodiscard]] const char* what() const noexcept override {
            return "Stored Value Patch would cause a Key Change";
        }
    };

private:
    friend class FacebookAdStore;

    UpperStrings upperCase;
    std::shared_ptr<FacebookAd> item;
    KeyType key;
};

class FacebookAdStore {
public:
    const StoredFacebookAd& Get(const StoredFacebookAd::KeyType & key) const;
    const StoredFacebookAd& Store(std::unique_ptr<FacebookAd> ad);

private:
    std::map<const StoredFacebookAd::KeyRef, std::unique_ptr<StoredFacebookAd>> ads;
    StoredFacebookAd NullAdd;
};

#endif //ELECTIONDATAANAL_FACEBOOKADSTORE_H
