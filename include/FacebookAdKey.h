//
// Created by lukeh on 01/11/2019.
//

#ifndef ELECTIONDATAANAL_FACEBOOKADKEY_H
#define ELECTIONDATAANAL_FACEBOOKADKEY_H
#include <FacebookAdStore.h>
#include <Index.h>
#include "../internal_includes/FacebookAdJSON.h"

class FacebookAdKey: public IndexKey<StoredFacebookAd, StoredFacebookAd::KeyType>
{
public:
    [[nodiscard]]
    const KeyType& GetKey(const StoredFacebookAd& item) const override final {
        return item.Key();
    }

    bool HasKey(const StoredFacebookAd& item, const std::string& key) const override final;

    std::string Serialize(
        const std::vector<std::string>& names,
        const std::vector<std::vector<KeyType>>& ids);

    void DeSerialize(
        const std::string& serialization,
        std::vector<std::string>& names,
        std::vector<std::vector<KeyType>>& ids);
};
#endif //ELECTIONDATAANAL_FACEBOOKADKEY_H
