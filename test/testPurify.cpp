//
// Created by lukeh on 04/11/2019.
//
#include "../internal_includes/SummmaryJSON.h"
#include "../internal_includes/ConfigPurify.h"
#include <gtest/gtest.h>
#include <AdDb.h>


class TPurify: public ::testing::Test {
public:
protected:
    void SetUp() {
    }
};

TEST_F(TPurify, BadCfg) {
    const std::string dbConfig = R"JSON(
        {
          "consituencies": [
            {
    )JSON";
    std::set<std::string> banList = {"the"};
    CfgPurify::FilteredKeysLists filtered;

    ASSERT_THROW(CfgPurify::RemoveBannedKeys(banList, dbConfig, filtered), CfgPurify::BadCfg);

}

TEST_F(TPurify, RemoveEnglishCons) {
    const std::string dbConfig = R"JSON(
        {
          "consituencies": [
            {
                 "id": "Search#0",
                 "keys": ["entity#0", "the"]
            }]
        }
    )JSON";

    std::set<std::string> banList = {"the"};

    CfgPurify::FilteredKeysLists filtered;

    AdDb db(CfgPurify::RemoveBannedKeys(banList, dbConfig, filtered));

    ASSERT_EQ(filtered.size(), 1);
    ASSERT_EQ(filtered["Search#0"].size(), 1);
    ASSERT_EQ(filtered["Search#0"][0], "the");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 1;
    ad->fundingEntity = "entity#0";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0")[0]->fundingEntity, "entity#0");

    ad = std::make_unique<FacebookAd>();
    ad->id = 2;
    ad->fundingEntity = "the";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0")[0]->fundingEntity, "entity#0");
}

TEST_F(TPurify, RemoveEnglishCons_TheHandling) {
    const std::string dbConfig = R"JSON(
        {
          "consituencies": [
            {
                 "id": "Search#0",
                 "keys": ["entity#0", "Box, The", "the box"]
            }]
        }
    )JSON";

    std::set<std::string> banList = {"box", "the"};

    CfgPurify::FilteredKeysLists filtered;

    AdDb db(CfgPurify::RemoveBannedKeys(banList, dbConfig, filtered));

    ASSERT_EQ(filtered.size(), 1);
    ASSERT_EQ(filtered["Search#0"].size(), 2);
    ASSERT_EQ(filtered["Search#0"][0], "Box, The");
    ASSERT_EQ(filtered["Search#0"][1], "the box");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 2;
    ad->fundingEntity = "entity#0";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0")[0]->fundingEntity, "entity#0");

    ad = std::make_unique<FacebookAd>();
    ad->id = 3;
    ad->fundingEntity = "Box, The";
    db.Store(std::move(ad));

    ad = std::make_unique<FacebookAd>();
    ad->id = 4;
    ad->fundingEntity = "the box";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0")[0]->fundingEntity, "entity#0");
}

TEST_F(TPurify, RemoveEnglishCons_RetainName) {
    const std::string dbConfig = R"JSON(
        {
          "consituencies": [
            {
                 "id": "Box",
                 "keys": ["Box", "duck box", "the duck"]
            }]
        }
    )JSON";

    std::set<std::string> banList = {"duck", "box", "the"};

    CfgPurify::FilteredKeysLists filtered;

    AdDb db(CfgPurify::RemoveBannedKeys(banList, dbConfig, filtered));

    ASSERT_EQ(filtered.size(), 1);
    ASSERT_EQ(filtered["Box"].size(), 1);
    ASSERT_EQ(filtered["Box"][0], "the duck");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 5;
    ad->fundingEntity = "duck box";
    db.Store(std::move(ad));

    ad = std::make_unique<FacebookAd>();
    ad->id = 6;
    ad->fundingEntity = "box";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Box").size(), 2);
    ASSERT_EQ(db.GetConstituency("Box")[0]->fundingEntity, "duck box");
    ASSERT_EQ(db.GetConstituency("Box")[1]->fundingEntity, "box");

    ad = std::make_unique<FacebookAd>();
    ad->id = 7;
    ad->fundingEntity = "the duck";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Box").size(), 2);
    ASSERT_EQ(db.GetConstituency("Box")[0]->fundingEntity, "duck box");
    ASSERT_EQ(db.GetConstituency("Box")[1]->fundingEntity, "box");
}

TEST_F(TPurify, RemoveEnglishCons_ChainedNames) {
    const std::string dbConfig = R"JSON(
        {
          "consituencies": [
            {
                 "id": "Box",
                 "keys": ["Box", "duck box"]
            }],
          "consituencies": [
            {
                 "id": "Box",
                 "keys": ["the duck"]
            }]
        }
    )JSON";

    std::set<std::string> banList = {"duck", "box", "the"};

    CfgPurify::FilteredKeysLists filtered;

    AdDb db(CfgPurify::RemoveBannedKeys(banList, dbConfig, filtered));

    ASSERT_EQ(filtered.size(), 1);
    ASSERT_EQ(filtered["Box"].size(), 1);
    ASSERT_EQ(filtered["Box"][0], "the duck");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 8;
    ad->fundingEntity = "duck box";
    db.Store(std::move(ad));

    ad = std::make_unique<FacebookAd>();
    ad->id = 9;
    ad->fundingEntity = "box";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Box").size(), 2);
    ASSERT_EQ(db.GetConstituency("Box")[0]->fundingEntity, "duck box");
    ASSERT_EQ(db.GetConstituency("Box")[1]->fundingEntity, "box");

    ad = std::make_unique<FacebookAd>();
    ad->id = 10;
    ad->fundingEntity = "the duck";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Box").size(), 2);
    ASSERT_EQ(db.GetConstituency("Box")[0]->fundingEntity, "duck box");
    ASSERT_EQ(db.GetConstituency("Box")[1]->fundingEntity, "box");
}

TEST_F(TPurify, RemoveDuplicates) {
    const std::string dbConfig = R"JSON(
        {
          "consituencies": [
            {
                 "id": "Search#0",
                 "keys": ["entity#0", "the"]
            }, {
                 "id": "Search#1",
                 "keys": ["entity#1", "tHe"]
            }, {
                 "id": "Search#0",
                 "keys": ["entity#0"]
            }]
        }
    )JSON";
    CfgPurify::FilteredKeysLists filtered;

    AdDb db(CfgPurify::RemoveDuplicates(dbConfig, filtered));

    ASSERT_EQ(filtered.size(), 2);
    ASSERT_EQ(filtered["Search#0"].size(), 1);
    ASSERT_EQ(filtered["Search#0"][0], "THE");
    ASSERT_EQ(filtered["Search#1"].size(), 1);
    ASSERT_EQ(filtered["Search#1"][0], "THE");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 10;
    ad->fundingEntity = "entity#0";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0")[0]->fundingEntity, "entity#0");

    ad = std::make_unique<FacebookAd>();
    ad->id = 11;
    ad->fundingEntity = "the";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0")[0]->fundingEntity, "entity#0");
}

TEST_F(TPurify, RemoveDuplicates_RetainName) {
    const std::string dbConfig = R"JSON(
        {
          "consituencies": [
            {
                 "id": "Search#0, Search#1",
                 "keys": ["Search#0", "search#1"]
            }, {
                 "id": "Search#2",
                 "keys": ["Search#1", "Search#2"]
            }]
        }
    )JSON";
    CfgPurify::FilteredKeysLists filtered;

    AdDb db(CfgPurify::RemoveDuplicates(dbConfig, filtered));

    ASSERT_EQ(filtered.size(), 1);
    ASSERT_EQ(filtered["Search#0, Search#1"].size(), 0);
    ASSERT_EQ(filtered["Search#2"].size(), 1);
    ASSERT_EQ(filtered["Search#2"][0], "SEARCH#1");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 12;
    ad->fundingEntity = "search#0";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0, Search#1").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0, Search#1")[0]->fundingEntity, "search#0");

    ASSERT_EQ(db.GetConstituency("Search#2").size(), 0);
}

TEST_F(TPurify, InvalidDictionaryPath) {
    ASSERT_THROW(CfgPurify::LoadDictionary("../NotAFile.cfg"), CfgPurify::BadFile);
}
TEST_F(TPurify, PurgeFromDictionaryFile) {
    CfgPurify::BanList banList = CfgPurify::LoadDictionary("../test/data/cfg/dictionary.cfg");

    const std::string dbConfig = R"JSON(
        {
          "consituencies": [
            {
                 "id": "Search#0",
                 "keys": ["they", "hello", "entity#0", "tHe"]
            }]
        }
    )JSON";
    CfgPurify::FilteredKeysLists filtered;
    AdDb db(CfgPurify::RemoveBannedKeys(banList, dbConfig, filtered));

    ASSERT_EQ(filtered.size(), 1);
    ASSERT_EQ(filtered["Search#0"].size(), 3);
    ASSERT_EQ(filtered["Search#0"][0], "they");
    ASSERT_EQ(filtered["Search#0"][1], "hello");
    ASSERT_EQ(filtered["Search#0"][2], "tHe");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 13;
    ad->fundingEntity = "entity#0";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0")[0]->fundingEntity, "entity#0");

    ad = std::make_unique<FacebookAd>();
    ad->id = 14;
    ad->fundingEntity = "the";
    db.Store(std::move(ad));

    ASSERT_EQ(db.GetConstituency("Search#0").size(), 1);
    ASSERT_EQ(db.GetConstituency("Search#0")[0]->fundingEntity, "entity#0");
}

