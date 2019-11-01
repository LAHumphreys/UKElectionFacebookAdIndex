//
// Created by lukeh on 02/11/2019.
//
#include <AdDb.h>

namespace {
    namespace Config {
        NewStringField(id);
        NewStringArrayField(keys);

        using Item = SimpleParsedJSON<id, keys>;
        NewObjectArray(consituencies, Item);

        using DbConfig = SimpleParsedJSON<consituencies>;
    }

    std::unique_ptr<AdDb::DbConfig> ParseConfig(const char* cfg) {
        std::unique_ptr<AdDb::DbConfig>  config;
        std::string error;
        Config::DbConfig configParser;
        std::vector<IndexConfig::Item> items;
        if (!configParser.Parse(cfg, error)) {
            throw AdDb::InvalidConfigError {error};
        } else if (!configParser.Supplied<Config::consituencies>()) {
            throw AdDb::InvalidConfigError {"'consituencies' list was not provided!"};
        } else {
            auto& cons = configParser.Get<Config::consituencies>();
            items.reserve(cons.size());

            for (auto& pItem: cons) {
                if (!pItem->Supplied<Config::id>()) {
                    throw AdDb::InvalidConfigError {"Every consituency must have an id"};
                } else if (!pItem->Supplied<Config::keys>()) {
                    std::string err = "Missing keys list for consituency: " + pItem->Get<Config::id>();
                    throw AdDb::InvalidConfigError {std::move(err)};
                } else {
                    items.push_back({pItem->Get<Config::id>(), pItem->Get<Config::keys>()});
                }
            }

            config = std::make_unique<AdDb::DbConfig>(std::move(items));
        }

        return config;
    }

}

AdDb::AdDb(const std::string& cfg) {
    std::shared_ptr<FacebookAdKey> facebookKey = std::make_shared<FacebookAdKey>();
    config = ParseConfig(cfg.c_str());
    consituencies = std::make_unique<FacebookAdIndex>(config->consituencies, facebookKey);
}

void AdDb::Store(std::unique_ptr<FacebookAd> ad) {
    const auto& storedAd = store.Store(std::move(ad));
    consituencies->Update(storedAd);

}

AdDb::FacebookAdList AdDb::GetConstituency(const std::string &name) const {
    FacebookAdList results;
    const auto& keys = consituencies->Get(name);
    results.reserve(keys.size());
    for(const auto& key: keys) {
        const auto& storedAd = store.Get(key);
        if (!storedAd.IsNull()) {
            results.emplace_back(storedAd.NewSharedRef());
        }
    }
    return results;
}

void AdDb::ForEachAdByConstituency(const AdDb::ForEachFacebookAd &cb) {
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

