//
// Created by lukeh on 01/11/2019.
//

#ifndef ELECTIONDATAANAL_INDEX_HPP
#define ELECTIONDATAANAL_INDEX_HPP

#include "Index.h"

template<class Key>
Index<Key>::Index(std::shared_ptr<IndexConfig> cfg, std::shared_ptr<Key> key)
   : config(std::move(cfg))
   , key(std::move(key))
   , matches(*config)
{
}
template<class Key>
Index<Key>::Index(std::shared_ptr<IndexConfig> cfg, std::shared_ptr<Key> key, const std::string &serialization)
  : Index(cfg, key)
{
    matches.DeSerialize(*key, serialization);
}


template<class Key>
const typename Index<Key>::MatchList& Index<Key>::Get(const std::string description) const
{
    const auto idx = matches.Find(description);
    if (idx != MatchListStore::npos) {
        return matches.Get(idx);
    } else {
        return emptySet;
    }
}

template<class Key>
void Index<Key>::Update(const typename Key::ItemType& update) {
    for (size_t i = 0; i < config->items.size(); ++i) {
        const auto& check = config->items[i];

        bool match = key->HasKey(update, check.keys);

        if (match) {
            _m.lock();
            matches.Get(i).emplace_back(key->GetKey(update));
            _m.unlock();
        }
    }

}


template<class Key>
std::string Index<Key>::Serialize() const {
    return matches.Serialize(*key);
}

template<class Key>
Index<Key>::MatchListStore::MatchListStore(IndexConfig &cfg) {
    lists.resize(cfg.items.size());
    for (size_t i = 0; i < cfg.items.size(); ++i) {
        idxs[cfg.items[i].id] = i;
    }
}

template<class Key>
size_t Index<Key>::MatchListStore::Find(const std::string &name) const {
    size_t idx = npos;
    auto it = idxs.find(name);
    if (it != idxs.end()) {
        idx = it->second;
    }
    return idx;
}

template<class Key>
typename Index<Key>::MatchList& Index<Key>::MatchListStore::Get(const size_t idx) {
    return lists[idx];
}

template<class Key>
const typename Index<Key>::MatchList& Index<Key>::MatchListStore::Get(const size_t idx) const {
    return lists[idx];
}

template<class Key>
std::string Index<Key>::MatchListStore::Serialize(Key &k) const {
    std::vector<std::string> names;
    names.resize(lists.size());
    for (const auto& pair: idxs) {
        names[pair.second] = pair.first;
    }
    return k.Serialize(names, lists);
}

template<class Key>
void Index<Key>::MatchListStore::DeSerialize(Key& k, const std::string &data)
{
    std::vector<std::string> names;
    k.DeSerialize(data, names, lists);

    std::map<std::string, size_t> oldIdxs;
    for (size_t i = 0; i < names.size(); ++i) {
        oldIdxs[names[i]] = i;
    }

    auto newIt = idxs.begin(), oldIt = oldIdxs.begin();
    for (;
         newIt != idxs.end() && oldIt != oldIdxs.end();
         ++newIt, ++oldIt)
    {
        if (newIt->first != oldIt->first) {
            throw ConfigChangedError{};
        }
    }
    if (newIt != idxs.end() || oldIt != oldIdxs.end()) {
        throw ConfigChangedError{};
    }

    idxs = std::move(oldIdxs);
}


#endif //ELECTIONDATAANAL_INDEX_HPP
