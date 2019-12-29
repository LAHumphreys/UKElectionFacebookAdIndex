//
// Created by lukeh on 01/11/2019.
//
#include "FacebookAdKey.h"
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/IndexJSON.h"

using namespace DbUtils;

namespace {
}

bool FacebookAdKey::HasKey(const StoredFacebookAd &item, const std::string &key) const {
    bool match = false;
    if (!item.IsNull()) {
        if (Search(item.CachedUppers().linkTitle, key)) {
            match = true;
        } else if (Search(item.CachedUppers().fundingEntity, key)) {
            match = true;
        } else if (Search(item.CachedUppers().pageName, key)) {
            match = true;
        } else if (Search(item.CachedUppers().body, key)) {
            match = true;
        } else if (Search(item.CachedUppers().linkDescription, key)) {
            match = true;
        } else if (Search(item.CachedUppers().linkCaption, key)) {
            match = true;
        }
    }
    return match;
}

bool FacebookAdKey::HasKey(const StoredFacebookAd &item, const std::vector<std::string> &keys) const {
    bool match = false;
    if (!item.IsNull()) {
        if (SearchForOneOf(item.CachedUppers().linkTitle.c_str(), keys)) {
            match = true;
        } else if (SearchForOneOf(item.CachedUppers().fundingEntity.c_str(), keys)) {
            match = true;
        } else if (SearchForOneOf(item.CachedUppers().pageName.c_str(), keys )) {
            match = true;
        } else if (SearchForOneOf(item.CachedUppers().body.c_str(), keys )) {
            match = true;
        } else if (SearchForOneOf(item.CachedUppers().linkDescription.c_str(), keys )) {
            match = true;
        } else if (SearchForOneOf(item.CachedUppers().linkCaption.c_str(), keys)) {
            match = true;
        }
    }
    return match;
}

std::string
FacebookAdKey::Serialize(const std::vector<std::string> &names, const std::vector<std::vector<KeyType>> &ids) {
    thread_local IndexJSON::JSON encoder;
    encoder.Clear();

    encoder.Get<IndexJSON::items>().resize(names.size());
    for (size_t i = 0; i < names.size() && i < ids.size(); ++i) {
        auto& next = *encoder.Get<IndexJSON::items>()[i];
        next.Get<IndexJSON::name>() = std::move(names[i]);
        next.Get<IndexJSON::keys>() = std::move(ids[i]);
    }

    return encoder.GetJSONString();
}

void FacebookAdKey::DeSerialize(
        const std::string &serialization,
        std::vector<std::string> &names,
        std::vector<std::vector<KeyType>> &ids)
{
    thread_local IndexJSON::JSON decoder;
    decoder.Clear();

    std::string error;
    names.clear();
    ids.clear();
    names.reserve(decoder.Get<IndexJSON::items>().size());
    ids.reserve(decoder.Get<IndexJSON::items>().size());
    if (decoder.Parse(serialization.c_str(), error)) {
        for (size_t i = 0; i < decoder.Get<IndexJSON::items>().size(); ++i) {
            auto& item = *decoder.Get<IndexJSON::items>()[i];
            names.emplace_back(std::move(item.Get<IndexJSON::name>()));
            ids.emplace_back(std::move(item.Get<IndexJSON::keys>()));
        }
    }

}

