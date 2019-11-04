//
// Created by lukeh on 03/11/2019.
//
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/SummmaryJSON.h"
#include <fstream>
#include <OSTools.h>
#include <FacebookParser.h>
#include <Reports.h>

using namespace DbUtils;

std::unique_ptr<AdDb> DbUtils::LoadDb(const std::string &cfgPath, const std::string &dataDir) {
    std::ifstream cfgFile(cfgPath);
    std::unique_ptr<AdDb> result;
    if (cfgFile.fail()) {
        throw DbUtils::NoSuchCfgFile{cfgPath};
    } else {
        std::string cfg((std::istreambuf_iterator<char>(cfgFile)), std::istreambuf_iterator<char>());
        result = std::make_unique<AdDb> (cfg);
    }

    FacebookAdParser parser;
    std::vector<FacebookAd> ads;
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
    for (auto& ad: ads) {
        result->Store(std::make_unique<FacebookAd>(std::move(ad)));
    }

    return result;
}

void DbUtils::DoConstituencyReport(Reports::Report& report, const std::string &basePath) {
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
            adsBuilder.Add("ad_creation_time", ad.ad->creationTime.ISO8601Timestamp());
            adsBuilder.Add("ad_creative_link_description", ad.ad->linkDescription);
            adsBuilder.Add("ad_creative_link_title", ad.ad->linkTitle);
            adsBuilder.Add("ad_creative_link_caption", ad.ad->linkCaption);
            adsBuilder.Add("ad_creative_body", ad.ad->body);
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
