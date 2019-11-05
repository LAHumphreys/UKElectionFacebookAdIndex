//
// Created by lukeh on 02/11/2019.
//
#include "../internal_includes/test_utils.h"
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/SummmaryJSON.h"
#include "../internal_includes/ReportAdsJSON.h"
#include <gtest/gtest.h>
#include <AdDb.h>
#include <fstream>

const std::string dbConfig = R"JSON(
{
  "consituencies": [
    {
         "id": "Search#0",
         "keys": ["entity#0"]
    }, {
         "id": "Search#1",
         "keys": ["entity#1"]
    }],
  "consituencies": [
    {
         "id": "Search#0",
         "keys": ["<Aditional Search>"]
    }, {
         "id": "Search#2",
         "keys": ["entity#DOES_NOT_EXIST"]
    }],
  "issues": [
    {
         "id": "Brexit",
         "keys": ["brexit"]
    }
   ]
}
)JSON";


class TDb: public ::testing::Test {
public:
    TDb()
       : theDb(dbConfig)
    {
        FacebookAd& ad = ads.emplace_back();
        ad.id = 0;
        ad.creationTime = nstimestamp::Time("2019-10-29T16:15:59+0000");
        ad.fundingEntity = "Entity#0";
        ad.pageName = "Page#0";
        ad.linkTitle = "Title#0";
        ad.linkCaption = "Caption#0";
        ad.linkDescription = "Description#0";
        ad.body = "Body#0";

        FacebookAd& ad1 = ads.emplace_back();
        ad1.id = 1;
        ad1.creationTime = nstimestamp::Time("2019-10-29T18:16:59+0000");
        ad1.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad1.fundingEntity = "Entity#1";
        ad1.pageName = "Page#1";
        ad1.linkTitle = "Title#1";
        ad1.linkCaption = "Caption#1";
        ad1.linkDescription = "Description#1";
        ad1.body = "Body#1";

        FacebookAd& ad2 = ads.emplace_back();
        ad2.id = 2;
        ad2.creationTime = nstimestamp::Time("2019-10-29T18:17:59+0000");
        ad1.deliveryStartTime = nstimestamp::Time("2019-10-29T18:18:60+0000");
        ad2.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad2.fundingEntity = "Entity#1";
        ad2.pageName = "Page#2";
        ad2.linkTitle = "Title#2";
        ad2.linkCaption = "Caption#2";
        ad2.linkDescription = "Description#2";
        ad2.body = "Brexit!";
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

TEST_F(TDb, SingleIssue) {
    auto matches = theDb.GetIssue("Brexit");
    ASSERT_EQ(matches.size(), 1);

    AssertEq(*matches[0], ads[2]);
}

TEST_F(TDb, ForEachConstituencyKey) {
    std::vector<std::string> cons;
    theDb.ForEachConsituency([&] (const std::string& name) -> auto {
        cons.push_back(name);
        return AdDb::DbScanOp::CONTINUE;
    });
    ASSERT_EQ(cons.size(), 3);

    ASSERT_EQ(cons[0], "Search#0");
    ASSERT_EQ(cons[1], "Search#1");
    ASSERT_EQ(cons[2], "Search#2");
}

TEST_F(TDb, ForEachConstituencyKey_Stop) {
    std::vector<std::string> cons;
    theDb.ForEachConsituency([&] (const std::string& name) -> auto {
        cons.push_back(name);
        return AdDb::DbScanOp::STOP;
    });
    ASSERT_EQ(cons.size(), 1);

    ASSERT_EQ(cons[0], "Search#0");
}

// Consituency Key already validates the loop logic,
// just validate our glue code is setting it up with the right index...
TEST_F(TDb, ForEachIssueKey) {
    std::vector<std::string> issues;
    theDb.ForEachIssue([&] (const std::string& name) -> auto {
        issues.push_back(name);
        return AdDb::DbScanOp::CONTINUE;
    });
    ASSERT_EQ(issues.size(), 1);

    ASSERT_EQ(issues[0], "Brexit");
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

TEST(DbUtilsTest, ConReport_Summary) {
    auto db = DbUtils::LoadDb("../test/data/cfg/woking_beeston.json", "../test/data/full_day_run");
    auto report = Reports::DoConsituencyReport(*db);

    DbUtils::WriteReport(*report, ".");

    std::ifstream summaryOutput("./Summary.json");
    ASSERT_FALSE(summaryOutput.fail());

    SummaryJSON::SummaryJSON summaryParser;
    std::string rawSummary((std::istreambuf_iterator<char>(summaryOutput)), std::istreambuf_iterator<char>());
    std::string error;
    ASSERT_TRUE(summaryParser.Parse(rawSummary.c_str(), error)) << error;

    ASSERT_EQ(summaryParser.Get<SummaryJSON::summary>().size(), 2);

    auto& beeston = summaryParser.Get<SummaryJSON::summary>()[0];
    auto& reportBeeston = (*report)["Beeston"];
    ASSERT_TRUE(beeston->Supplied<SummaryJSON::totalAds>());
    ASSERT_EQ(beeston->Get<SummaryJSON::totalAds>(), reportBeeston.summary.count);
    ASSERT_TRUE(beeston->Supplied<SummaryJSON::guestimateImpressions>());
    ASSERT_EQ(beeston->Get<SummaryJSON::guestimateImpressions>(), reportBeeston.summary.estImpressions);
    ASSERT_TRUE(beeston->Supplied<SummaryJSON::guestimateSpendGBP>());
    ASSERT_EQ(beeston->Get<SummaryJSON::guestimateSpendGBP>(), reportBeeston.summary.estSpend);

    auto& woking = summaryParser.Get<SummaryJSON::summary>()[1];
    auto& reportWoking = (*report)["Woking"];
    ASSERT_TRUE(woking->Supplied<SummaryJSON::totalAds>());
    ASSERT_EQ(woking->Get<SummaryJSON::totalAds>(), reportWoking.summary.count);
    ASSERT_TRUE(woking->Supplied<SummaryJSON::guestimateImpressions>());
    ASSERT_EQ(woking->Get<SummaryJSON::guestimateImpressions>(), reportWoking.summary.estImpressions);
    ASSERT_TRUE(woking->Supplied<SummaryJSON::guestimateSpendGBP>());
    ASSERT_EQ(woking->Get<SummaryJSON::guestimateSpendGBP>(), reportWoking.summary.estSpend);
}

TEST(DbUtilsTest, ConReport_NoAds) {
    auto db = DbUtils::LoadDb("../test/data/cfg/woking_beeston.json", "../test/data/full_day_run");
    auto report = Reports::DoConsituencyReport(*db);

    DbUtils::WriteReport(*report, ".");

    std::ifstream beestonOutput("./Beeston.json");
    ASSERT_FALSE(beestonOutput.fail());
    ReportJSON::ReportJSON adsParser;
    std::string rawSummary((std::istreambuf_iterator<char>(beestonOutput)), std::istreambuf_iterator<char>());
    std::string error;
    ASSERT_TRUE(adsParser.Parse(rawSummary.c_str(), error)) << error;
    ASSERT_EQ(adsParser.Get<ReportJSON::data>().size(), 0);
}

TEST(DbUtilsTest, ConReport_Ads) {
    auto db = DbUtils::LoadDb("../test/data/cfg/woking_beeston.json", "../test/data/full_day_run");
    auto report = Reports::DoConsituencyReport(*db);
    auto& reportWoking = (*report)["Woking"];

    DbUtils::WriteReport(*report, ".");

    std::ifstream beestonOutput("./Woking.json");
    ASSERT_FALSE(beestonOutput.fail());
    ReportJSON::ReportJSON adsParser;
    std::string rawSummary((std::istreambuf_iterator<char>(beestonOutput)), std::istreambuf_iterator<char>());
    std::string error;
    ASSERT_TRUE(adsParser.Parse(rawSummary.c_str(), error)) << error;
    ASSERT_EQ(adsParser.Get<ReportJSON::data>().size(), reportWoking.ads.size());

    for (size_t i = 0; i < adsParser.Get<ReportJSON::data>().size(); ++i) {
        auto& fileAd = *adsParser.Get<ReportJSON::data>()[i];
        auto& ad = *reportWoking.ads[i].ad;
        ASSERT_EQ(fileAd.Get<ReportJSON::funding_entity>(), ad.fundingEntity);
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_delivery_start_time>(), ad.deliveryStartTime.ISO8601Timestamp());
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_delivery_end_time>(), ad.deliveryEndTime.ISO8601Timestamp());
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creation_time>(), ad.creationTime.ISO8601Timestamp());
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creative_link_description>(), ad.linkDescription);
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creative_link_title>(), ad.linkTitle);
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creative_link_caption>(), ad.linkCaption );
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creative_body>(), ad.body );
        ASSERT_EQ(fileAd.Get<ReportJSON::guestimateSpendGBP>(), reportWoking.ads[i].guestimateSpend );
        ASSERT_EQ(fileAd.Get<ReportJSON::guestimateImpressions>(), reportWoking.ads[i].guestimateImpressions );
    }
}

TEST(DbUtilsTest, ConReport_Ads_RedactedMode) {
    auto db = DbUtils::LoadDb("../test/data/cfg/woking_beeston.json", "../test/data/full_day_run");
    auto report = Reports::DoConsituencyReport(*db);
    auto& reportWoking = (*report)["Woking"];

    DbUtils::WriteReport(*report, ".", DbUtils::WriteMode::REDACTED);

    std::ifstream beestonOutput("./Woking.json");
    ASSERT_FALSE(beestonOutput.fail());
    ReportJSON::ReportJSON adsParser;
    std::string rawSummary((std::istreambuf_iterator<char>(beestonOutput)), std::istreambuf_iterator<char>());
    std::string error;
    ASSERT_TRUE(adsParser.Parse(rawSummary.c_str(), error)) << error;
    ASSERT_EQ(adsParser.Get<ReportJSON::data>().size(), reportWoking.ads.size());

    for (size_t i = 0; i < adsParser.Get<ReportJSON::data>().size(); ++i) {
        auto& fileAd = *adsParser.Get<ReportJSON::data>()[i];
        auto& ad = *reportWoking.ads[i].ad;
        ASSERT_EQ(fileAd.Get<ReportJSON::funding_entity>(), ad.fundingEntity);
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_delivery_start_time>(), ad.deliveryStartTime.ISO8601Timestamp());
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_delivery_end_time>(), ad.deliveryEndTime.ISO8601Timestamp());
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creation_time>(), ad.creationTime.ISO8601Timestamp());
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creative_link_description>(), DbUtils::REDACTED_TEXT);
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creative_link_title>(), DbUtils::REDACTED_TEXT);
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creative_link_caption>(), DbUtils::REDACTED_TEXT);
        ASSERT_EQ(fileAd.Get<ReportJSON::ad_creative_body>(), DbUtils::REDACTED_TEXT);
        ASSERT_EQ(fileAd.Get<ReportJSON::guestimateSpendGBP>(), reportWoking.ads[i].guestimateSpend );
        ASSERT_EQ(fileAd.Get<ReportJSON::guestimateImpressions>(), reportWoking.ads[i].guestimateImpressions );
    }
}
