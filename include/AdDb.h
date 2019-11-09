//
// Created by lukeh on 02/11/2019.
//

#ifndef ELECTIONDATAANAL_ADDB_H
#define ELECTIONDATAANAL_ADDB_H
#include <memory>
#include <FacebookAd.h>
#include <FacebookAdKey.h>
#include <Index.h>
#include <memory>
#include <functional>

class AdDb {
public:
    enum class DbScanOp {
        CONTINUE,
        STOP
    };
    AdDb(const std::string& cfg);

    using FacebookAdIndex = Index<FacebookAdKey>;
    using FacebookAdList  = std::vector<std::shared_ptr<const FacebookAd>>;
    using ForEachFacebookAd =
            std::function<DbScanOp (const IndexConfig::Item&, std::shared_ptr<const FacebookAd> ad)>;
    using ForEachItemDefn = std::function<DbScanOp (const std::string& name)>;
    void Store(std::unique_ptr<FacebookAd> ad);

    [[nodiscard]] FacebookAdList GetConstituency(const std::string& name) const;
    [[nodiscard]] FacebookAdList GetIssue(const std::string& name) const;

    void ForEachAdByConstituency(const ForEachFacebookAd& cb) const;
    void ForEachConsituency(const ForEachItemDefn& cb) const;
    void ForEachIssue(const ForEachItemDefn& cb) const;

    struct InvalidConfigError: std::exception {
        InvalidConfigError(std::string err): error(std::move(err)) {}
        const char * what() const noexcept {return error.c_str();}
        std::string error;
    };

    struct DbConfig {
        using ItemList = std::vector<IndexConfig::Item>;
        std::shared_ptr<IndexConfig> consituencies;
        std::shared_ptr<IndexConfig> issues;
        nstimestamp::Time startTimeCutOff;
        nstimestamp::Time endTimeCutOff;
    };
private:
    FacebookAdList Get(const FacebookAdIndex& idx, const std::string& key) const;
    void ForEach(IndexConfig& idx, const AdDb::ForEachItemDefn &cb) const;
    FacebookAdStore store;
    std::unique_ptr<FacebookAdIndex> consituencies;
    std::unique_ptr<FacebookAdIndex> issues;

    std::unique_ptr<DbConfig> config;
};

#endif //ELECTIONDATAANAL_ADDB_H
