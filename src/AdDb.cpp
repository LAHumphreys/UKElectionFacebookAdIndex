//
// Created by lukeh on 02/11/2019.
//
#include <AdDb.h>
#include <iostream>

namespace {
    namespace Config {
        NewStringField(id);
        NewStringArrayField(keys);

        using Item = SimpleParsedJSON<id, keys>;
        NewObjectArray(consituencies, Item);
        NewObjectArray(issues, Item);

        using DbConfig = SimpleParsedJSON<consituencies, issues>;
    }

    template<class ItemGroup>
    std::vector<IndexConfig::Item> LoadItems(Config::DbConfig& config) {
        std::vector<IndexConfig::Item> items;
        auto &cons = config.Get<ItemGroup>();
        items.reserve(cons.size());

        for (auto &pItem: cons) {
            if (!pItem->template Supplied<Config::id>()) {
                throw AdDb::InvalidConfigError{"Every consituency must have an id"};
            } else if (!pItem->template Supplied<Config::keys>()) {
                std::string err = "Missing keys list for consituency: " + pItem->template Get<Config::id>();
                throw AdDb::InvalidConfigError{std::move(err)};
            } else {
                bool found = false;
                const std::string& id = pItem->template Get<Config::id>();
                for (auto it = items.begin(); !found && it != items.end(); ++it) {
                    IndexConfig::Item& item = *it;
                    if (item.id == id) {
                        found = true;
                        for  (auto& k: pItem->template Get<Config::keys>()) {
                            item.keys.push_back(std::move(k));
                        }
                    }
                }
                if (!found) {
                    items.push_back({id, pItem->template Get<Config::keys>()});
                }
            }
        }

        return items;
    }


    std::unique_ptr<AdDb::DbConfig> ParseConfig(const char* cfg) {
        std::unique_ptr<AdDb::DbConfig>  config;
        std::string error;
        Config::DbConfig configParser;
        std::vector<IndexConfig::Item> cons;
        std::vector<IndexConfig::Item> issues;
        if (!configParser.Parse(cfg, error)) {
            throw AdDb::InvalidConfigError {error};
        }
        if (!configParser.Supplied<Config::consituencies>()) {
            throw AdDb::InvalidConfigError {"'consituencies' list was not provided!"};
        } else {
            cons = LoadItems<Config::consituencies>(configParser);
        }

        if (configParser.Supplied<Config::issues>()) {
            issues = LoadItems<Config::issues>(configParser);
        }

        config = std::make_unique<AdDb::DbConfig>(std::move(cons), std::move(issues));

        return config;
    }

}

AdDb::AdDb(const std::string& cfg) {
    std::shared_ptr<FacebookAdKey> facebookKey = std::make_shared<FacebookAdKey>();
    config = ParseConfig(cfg.c_str());
    consituencies = std::make_unique<FacebookAdIndex>(config->consituencies, facebookKey);
    issues = std::make_unique<FacebookAdIndex>(config->issues, facebookKey);
}

void AdDb::Store(std::unique_ptr<FacebookAd> ad) {
    const auto& storedAd = store.Store(std::move(ad));
    consituencies->Update(storedAd);
    issues->Update(storedAd);

}
AdDb::FacebookAdList AdDb::Get(const FacebookAdIndex& idx, const std::string& name) const {
    FacebookAdList results;
    const auto& keys = idx.Get(name);
    results.reserve(keys.size());
    for(const auto& key: keys) {
        const auto& storedAd = store.Get(key);
        if (!storedAd.IsNull()) {
            results.emplace_back(storedAd.NewSharedRef());
        }
    }
    return results;

}

AdDb::FacebookAdList AdDb::GetConstituency(const std::string &name) const {
    return Get(*consituencies, name);
}

AdDb::FacebookAdList AdDb::GetIssue(const std::string &name) const {
    return Get(*issues, name);
}

void AdDb::ForEachAdByConstituency(const AdDb::ForEachFacebookAd &cb) const {
    bool continueScan = true;
    const auto& cons = config->consituencies->items;
    for (auto it = cons.begin(); continueScan && it != cons.end(); ++it) {
        const auto& con = *it;
        const auto& keys = consituencies->Get(con.id);
        for (auto kit = keys.begin(); continueScan && kit!=keys.end(); ++kit) {
            const auto& key = *kit;
            const auto& storedAd = store.Get(key);
            if (!storedAd.IsNull()) {
                switch(cb(con, storedAd.NewSharedRef())) {
                    case DbScanOp::CONTINUE:
                        continueScan = true;
                        break;
                    case DbScanOp::STOP:
                        continueScan = false;
                        break;
                }
            }
        }
    }
}

void AdDb::ForEach(IndexConfig& idx, const AdDb::ForEachItemDefn &cb) const {
    for (auto& cfg: idx.items) {
        switch(cb(cfg.id)) {
            case DbScanOp::CONTINUE:
                break;
            case DbScanOp::STOP:
                return;
        }
    }
}

void AdDb::ForEachConsituency(const AdDb::ForEachItemDefn &cb) const {
    ForEach(*config->consituencies, cb);
}

void AdDb::ForEachIssue(const AdDb::ForEachItemDefn &cb) const {
    ForEach(*config->issues, cb);
}


