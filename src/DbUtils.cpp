//
// Created by lukeh on 03/11/2019.
//
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/SummmaryJSON.h"
#include <fstream>
#include <OSTools.h>
#include <FacebookParser.h>
#include <Reports.h>
#include <logger.h>

using namespace DbUtils;
using namespace nstimestamp;

namespace {
    constexpr bool IsWordChar(char c) {
        if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            return true;
        } else {
            return false;
        }
    }
}
bool DbUtils::Search(const std::string& toSearch, const std::string& key) {
    size_t pos = 0;
    bool found = false;
    while (!found && pos != std::string::npos) {
        pos = toSearch.find(key, pos);

        bool match = true;
        const size_t next = pos + key.size();

        if (pos == std::string::npos) {
            match = false;
        }
        if (match && next < toSearch.size()) {
            char nchar = toSearch[next];
            match = !IsWordChar(nchar);
        }

        if (match && pos > 0) {
            char pchar = toSearch[pos-1];
            match = !IsWordChar(pchar);
        }

        if (match) {
            found = true;
        } else if (pos != std::string::npos) {
            ++pos;
        }
    }

    return found;
}

std::unique_ptr<AdDb>
DbUtils::LoadDb(const std::string &cfgPath, const std::string &dataDir, const std::string &dbStartState) {
    Time start;
    std::ifstream cfgFile(cfgPath);
    std::unique_ptr<AdDb> result;
    if (cfgFile.fail()) {
        throw DbUtils::NoSuchCfgFile{cfgPath};
    } else {
        std::string cfg((std::istreambuf_iterator<char>(cfgFile)), std::istreambuf_iterator<char>());
        if (dbStartState != "") {
            std::ifstream dbFile(dbStartState);
            std::string dbData((std::istreambuf_iterator<char>(dbFile)), std::istreambuf_iterator<char>());
            AdDb::Serialization data;
            data.json = std::move(dbData);
            result = std::make_unique<AdDb> (cfg, data);
        } else {
            result = std::make_unique<AdDb> (cfg);
        }
    }

    if (dataDir != "") {
        FacebookAdParser parser;
        std::vector<std::unique_ptr<FacebookAd>> ads;
        auto files = OS::Glob(dataDir + "/*");
        for (const std::string& path: files) {
            std::ifstream file(path);
            if (parser.Parse(file, ads) != FacebookAdParser::ParseResult::VALID) {
                throw DbUtils::BadData{};
            }
        }

        if (ads.size() == 0) {
            throw DbUtils::NoData{};
        }

        Time loadEnd;
        SLOG_FROM(LOG_OVERVIEW, __func__,
                  "Loaded " << ads.size() << " ads in " << loadEnd.DiffSecs(start) << "s")

        SLOG_FROM(LOG_OVERVIEW, __func__, "Building indexes...");

        size_t i = 0;
        Time lastReport;
        for (auto& ad: ads) {
            result->Store(std::move(ad));

            if (i %10 == 0) {
                Time now;
                if (now.DiffSecs(lastReport) >= 10) {
                    SLOG_FROM(LOG_OVERVIEW, __func__,
                              "Indexed " << i << " of " << ads.size() << " ads in " << now.DiffSecs(loadEnd) << "s")
                    lastReport = now;
                }
            }

            ++i;
        }

        Time storeEnd;
        SLOG_FROM(LOG_OVERVIEW, __func__,
                  "Indexed " << ads.size() << " ads in " << storeEnd.DiffSecs(loadEnd) << "s")

    }
    return result;
}

std::unique_ptr<AdDb> DbUtils::LoadDb(const std::string &cfgPath, const std::string &dataDir) {
    return LoadDb(cfgPath, dataDir, "");
}

void DbUtils::WriteReport(Reports::Report& report, const std::string &basePath, WriteMode mode) {
    std::fstream summaryFile(basePath + "/Summary.json", std::ios_base::out);
    SimpleJSONBuilder summaryBuilder;
    summaryBuilder.StartArray("summary");
    for (const auto &item: report) {
        summaryBuilder.StartAnonymousObject();
        summaryBuilder.Add("name", item.first);
        summaryBuilder.Add("totalAds", item.second.summary.count);
        summaryBuilder.Add("guestimateImpressions", item.second.summary.estImpressions);
        summaryBuilder.Add("guestimateSpendGBP", item.second.summary.estSpend);
        summaryBuilder.EndObject();

        std::fstream adsFile(basePath + "/" + item.first + ".json", std::ios_base::out);
        SimpleJSONBuilder adsBuilder;
        adsBuilder.StartArray("data");
        for (const auto& ad: item.second.ads) {
            adsBuilder.StartAnonymousObject();
            adsBuilder.Add("funding_entity", ad.ad->fundingEntity);
            adsBuilder.Add("ad_delivery_start_time", ad.ad->deliveryStartTime.ISO8601Timestamp());
            adsBuilder.Add("ad_delivery_end_time", ad.ad->deliveryEndTime.ISO8601Timestamp());
            adsBuilder.Add("ad_creation_time", ad.ad->creationTime.ISO8601Timestamp());
            if (mode == WriteMode::REDACTED) {
                const std::string redacted(REDACTED_TEXT);
                adsBuilder.Add("ad_creative_link_description", redacted);
                adsBuilder.Add("ad_creative_link_title", redacted);
                adsBuilder.Add("ad_creative_link_caption", redacted);
                adsBuilder.Add("ad_creative_body", redacted);
            } else {
                adsBuilder.Add("ad_creative_link_description", ad.ad->linkDescription);
                adsBuilder.Add("ad_creative_link_title", ad.ad->linkTitle);
                adsBuilder.Add("ad_creative_link_caption", ad.ad->linkCaption);
                adsBuilder.Add("ad_creative_body", ad.ad->body);
            }
            adsBuilder.Add("guestimateImpressions", ad.guestimateImpressions);
            adsBuilder.Add("guestimateSpendGBP", ad.guestimateSpend);
            adsBuilder.EndObject();
        }
        adsBuilder.EndArray();
        adsFile << adsBuilder.GetAndClear() << std::endl;
        adsFile.close();
    }
    summaryBuilder.EndArray();
    summaryFile << summaryBuilder.GetAndClear() << std::endl;
    summaryFile.close();
}

void DbUtils::WriteDbToDisk(AdDb &db, const std::string &fname) {
    std::fstream summaryFile(fname, std::ios_base::out);
    summaryFile << db.Serialize().json;
    summaryFile.close();
}
