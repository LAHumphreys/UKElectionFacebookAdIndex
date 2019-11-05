//
// Created by lukeh on 01/11/2019.
//

#ifndef ELECTIONDATAANAL_INDEX_H
#define ELECTIONDATAANAL_INDEX_H

#include <string>
#include <SimpleJSON.h>
#include <IndexConfig.h>

template <class Item, class Key>
class IndexKey {
public:
    using ItemType = Item;
    using KeyType = Key;

    virtual ~IndexKey() = default;

    virtual const KeyType& GetKey(const Item& item) const = 0;

    virtual bool HasKey(const Item& item, const std::string& key) const = 0;
private:
};

template<class Key>
class Index {
public:
    using MatchList = std::vector<typename Key::KeyType>;

    Index (
        std::shared_ptr<IndexConfig> config,
        std::shared_ptr<Key>  key);

    const MatchList& Get(const std::string description) const;

    void Update(const typename Key::ItemType& update);

private:
    const MatchList emptySet = {};
    std::shared_ptr<IndexConfig> config;
    std::shared_ptr<Key>  key;

    std::map<std::string, MatchList> matches;
};

#include <Index.hpp>
#endif //ELECTIONDATAANAL_INDEX_H
