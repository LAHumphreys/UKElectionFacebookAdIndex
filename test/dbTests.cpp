//
// Created by lukeh on 02/11/2019.
//
#include "../internal_includes/test_utils.h"
#include "../internal_includes/DbUtils.h"
#include <gtest/gtest.h>
#include <AdDb.h>

const std::string dbConfig = R"JSON(
{
  "consituencies": [
    {
         "id": "Search#0",
         "keys": ["entity#0"]
    }, {
         "id": "Search#1",
         "keys": ["entity#1"]
    }]
}
)JSON";


class TDb: public ::testing::Test {
public:
    TDb()
       : theDb(dbConfig)
    {
        FacebookAd& ad = ads.emplace_back();
        ad.creationTime = nstimestamp::Time("2019-10-29T16:15:59+0000");
        ad.fundingEntity = "Entity#0";
        ad.pageName = "Page#0";
        ad.linkTitle = "Title#0";
        ad.linkCaption = "Caption#0";
        ad.linkDescription = "Description#0";
        ad.body = "Body#0";

        FacebookAd& ad1 = ads.emplace_back();
        ad1.creationTime = nstimestamp::Time("2019-10-29T18:16:59+0000");
        ad1.fundingEntity = "Entity#1";
        ad1.pageName = "Page#1";
        ad1.linkTitle = "Title#1";
        ad1.linkCaption = "Caption#1";
        ad1.linkDescription = "Description#1";
        ad1.body = "Body#1";

        FacebookAd& ad2 = ads.emplace_back();
        ad2.creationTime = nstimestamp::Time("2019-10-29T18:17:59+0000");
        ad2.fundingEntity = "Entity#1";
        ad2.pageName = "Page#2";
        ad2.linkTitle = "Title#2";
        ad2.linkCaption = "Caption#2";
        ad2.linkDescription = "Description#2";
        ad2.body = "Body#2";
    }
protected:
    void SetUp() {
        for (const auto& ad: ads) {
            theDb.Store(std::make_unique<FacebookAd>(ad));
        }
    }

    AdDb                    theDb;
    std::vector<FacebookAd> ads;
};

TEST_F(TDb, SingleConstituency) {
    auto matches = theDb.GetConstituency("Search#1");
    ASSERT_EQ(matches.size(), 2);

    AssertEq(*matches[0], ads[1]);
    AssertEq(*matches[1], ads[2]);
}

TEST_F(TDb, ForEachConstituency) {
    std::vector<std::shared_ptr<const FacebookAd>> matches;
    theDb.ForEachAdByConstituency([&](const auto& key, std::shared_ptr<const FacebookAd> ad) -> auto {
        matches.emplace_back(std::move(ad));
        return AdDb::DbScanOp::CONTINUE;
    });
    ASSERT_EQ(matches.size(), 3);

    AssertEq(*matches[0], ads[0]);
    AssertEq(*matches[1], ads[1]);
    AssertEq(*matches[2], ads[2]);
}

TEST_F(TDb, ForEachConstituency_StopAfterGroup) {
    std::vector<std::shared_ptr<const FacebookAd>> matches;
    theDb.ForEachAdByConstituency([&](const auto& key, std::shared_ptr<const FacebookAd> ad) -> auto {
        matches.emplace_back(std::move(ad));
        return AdDb::DbScanOp::STOP;
    });
    ASSERT_EQ(matches.size(), 1);

    AssertEq(*matches[0], ads[0]);
}

TEST_F(TDb, ForEachConstituency_StopMidGroup) {
    std::vector<std::shared_ptr<const FacebookAd>> matches;
    theDb.ForEachAdByConstituency([&](const IndexConfig::Item& key, std::shared_ptr<const FacebookAd> ad) -> auto {
        matches.emplace_back(std::move(ad));
        return (key.id == "Search#1"? AdDb::DbScanOp::STOP : AdDb::DbScanOp::CONTINUE);
    });
    ASSERT_EQ(matches.size(), 2);

    AssertEq(*matches[0], ads[0]);
    AssertEq(*matches[1], ads[1]);
}

TEST(DbUtilsTest, NoSuchCfgFile) {
    ASSERT_THROW(DbUtils::LoadDb("../test/data/not_a_file", "../test/data/full_day_run"), DbUtils::NoSuchCfgFile);
}

TEST(DbUtilsTest, InvalidCfgFile) {
    ASSERT_THROW(DbUtils::LoadDb("../test/data/cfg/invalid.json", "../test/data/full_day_run"), AdDb::InvalidConfigError);
}

TEST(DbUtilsTest, NoData) {
    ASSERT_THROW(DbUtils::LoadDb("../test/data/cfg/woking.json", "../test/data/not_a_dir"), DbUtils::NoData);
}

TEST(DbUtilsTest, WokingData) {
    auto db = DbUtils::LoadDb("../test/data/cfg/woking.json", "../test/data/full_day_run");

    auto results = db->GetConstituency("Woking");

    ASSERT_EQ(results.size(), 2);

    const auto& libDemAd = *results[0];
    const auto& conAd = *results[1];

    // Whilst sort order is LOCALE defined, if someone's system is so obtuse as to sort 2 before 1, they're
    // welcome to pull request a fix for this test only problem
    ASSERT_EQ(libDemAd.fundingEntity, "Woking Liberal Democrats");
    ASSERT_EQ(conAd.fundingEntity, "Woking Conservative Association");
}
