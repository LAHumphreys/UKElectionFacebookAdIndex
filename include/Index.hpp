//
// Created by lukeh on 01/11/2019.
//

#ifndef ELECTIONDATAANAL_INDEX_HPP
#define ELECTIONDATAANAL_INDEX_HPP

#include "Index.h"

template<class Key>
Index<Key>::Index(std::shared_ptr<IndexConfig> config, std::shared_ptr<Key> key)
   : config(std::move(config))
   , key(std::move(key))
{
}

template<class Key>
const typename Index<Key>::MatchList& Index<Key>::Get(const std::string description) const
{
    auto it = matches.find(description);
    if (it != matches.end()) {
        return it->second;
    } else {
        return emptySet;
    }
}

template<class Key>
void Index<Key>::Update(const typename Key::ItemType& update) {
    for (const auto& check: config->items) {
        bool match = false;
        for (auto it = check.keys.begin();
             !match && it != check.keys.end();
             ++it )
        {
            const std::string& searchString = *it;
            if (key->HasKey(update, searchString)) {
                match = true;
            }
        }
        if (match) {
            matches[check.id].emplace_back(key->GetKey(update));
        }
    }

}

#endif //ELECTIONDATAANAL_INDEX_HPP
