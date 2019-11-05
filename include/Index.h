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

    class MatchListStore {
    public:
        constexpr static size_t npos = std::numeric_limits<size_t>::max();

        MatchListStore(IndexConfig& cfg);

        size_t Find(const std::string& name) const;
        MatchList& Get(const size_t idx);
        const MatchList& Get(const size_t idx) const;
    private:
        std::map<std::string, size_t> idxs;
        std::vector<MatchList> lists;
    };
    MatchListStore matches;
};

#include <Index.hpp>
#endif //ELECTIONDATAANAL_INDEX_H
