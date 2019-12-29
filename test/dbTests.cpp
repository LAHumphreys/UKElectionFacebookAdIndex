//
// Created by lukeh on 02/11/2019.
//
#include "../internal_includes/test_utils.h"
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/SummmaryJSON.h"
#include "../internal_includes/ReportAdsJSON.h"
#include "../internal_includes/ConfigParser.h"
#include "../internal_includes/test_BreakdownParser.h"
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
        ad2.deliveryStartTime = nstimestamp::Time("2019-10-29T18:18:60+0000");
        ad2.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad2.fundingEntity = "Entity#1";
        ad2.pageName = "Page#2";
        ad2.linkTitle = "Title#2";
        ad2.linkCaption = "Caption#2";
        ad2.linkDescription = "Description#2";
        ad2.body = "Brexit!";

        FacebookAd& ad3 = ads.emplace_back();
        ad3.id = 3;
    }
protected:
    void SetUp() {
        for (const auto& ad: ads) {
            theDb.Store(std::make_unique<FacebookAd>(ad));
        }
        std::string error;
        ASSERT_TRUE(pconfig.Parse(dbConfig.c_str(), error)) << error;
    }

    AdDb                    theDb;
    std::vector<FacebookAd> ads;
    Config::DbConfig        pconfig;
};

TEST_F(TDb, SingleConstituency) {
    auto matches = theDb.GetConstituency("Search#1");
    ASSERT_EQ(matches.size(), 2);

    AssertEq(*matches[0], ads[1]);
    AssertEq(*matches[1], ads[2]);
}

TEST_F(TDb, StartTimeFilter) {
    const std::string timeConfig = R"JSON(
    {
      "startingCutOff": "2019-10-29T12:00:00+0000",
      "consituencies": [{
             "id": "Search#0",
             "keys": ["entity#0"]
        }]
    })JSON";
    auto before = std::make_unique<FacebookAd>(ads[0]);
    before->id = 1;
    auto at = std::make_unique<FacebookAd>(ads[0]);
    at->id = 2;
    auto after = std::make_unique<FacebookAd>(ads[0]);
    after->id = 3;
    before->deliveryStartTime = nstimestamp::Time("2019-10-29T00:00:00+0000");
    at->deliveryStartTime = nstimestamp::Time("2019-10-29T12:00:00+0000");
    after->deliveryStartTime = nstimestamp::Time("2019-10-30T00:00:00+0000");

    AdDb   timeFilterDb(timeConfig);
    timeFilterDb.Store(std::make_unique<FacebookAd>(*before));
    timeFilterDb.Store(std::make_unique<FacebookAd>(*at));
    timeFilterDb.Store(std::make_unique<FacebookAd>(*after));

    auto matches = timeFilterDb.GetConstituency("Search#0");
    ASSERT_EQ(matches.size(), 2);

    AssertEq(*matches[0], *at);
    AssertEq(*matches[1], *after);
}

TEST_F(TDb, EndTimeFilter) {
    const std::string timeConfig = R"JSON(
    {
      "endCutOff": "2019-10-29T12:00:00+0000",
      "consituencies": [{
             "id": "Search#0",
             "keys": ["entity#0"]
        }]
    })JSON";
    auto before = std::make_unique<FacebookAd>(ads[0]);
    before->id = 1;
    auto at = std::make_unique<FacebookAd>(ads[0]);
    at->id = 2;
    auto after = std::make_unique<FacebookAd>(ads[0]);
    after->id = 3;
    before->deliveryEndTime = nstimestamp::Time("2019-10-29T00:00:00+0000");
    at->deliveryEndTime = nstimestamp::Time("2019-10-29T12:00:00+0000");
    after->deliveryEndTime = nstimestamp::Time("2019-10-30T00:00:00+0000");

    AdDb   timeFilterDb(timeConfig);
    timeFilterDb.Store(std::make_unique<FacebookAd>(*before));
    timeFilterDb.Store(std::make_unique<FacebookAd>(*at));
    timeFilterDb.Store(std::make_unique<FacebookAd>(*after));

    auto matches = timeFilterDb.GetConstituency("Search#0");
    ASSERT_EQ(matches.size(), 2);

    AssertEq(*matches[0], *at);
    AssertEq(*matches[1], *after);
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

TEST_F(TDb, ForEach) {
    std::vector<std::shared_ptr<const FacebookAd>> matches;
    theDb.ForEachAd([&](std::shared_ptr<const FacebookAd> ad) -> auto {
        matches.emplace_back(std::move(ad));
        return AdDb::DbScanOp::CONTINUE;
    });
    ASSERT_EQ(matches.size(), 4);

    AssertEq(*matches[0], ads[0]);
    AssertEq(*matches[1], ads[1]);
    AssertEq(*matches[2], ads[2]);
    AssertEq(*matches[3], ads[3]);
}

TEST_F(TDb, ForEach_Stop) {
    std::vector<std::shared_ptr<const FacebookAd>> matches;
    size_t i = 0;
    theDb.ForEachAd([&](std::shared_ptr<const FacebookAd> ad) -> auto {
        ++i;
        matches.emplace_back(std::move(ad));
        if (i < 2) {
            return AdDb::DbScanOp::CONTINUE;
        } else {
            return AdDb::DbScanOp::STOP;
        }
    });
    ASSERT_EQ(matches.size(), 2);

    AssertEq(*matches[0], ads[0]);
    AssertEq(*matches[1], ads[1]);
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

TEST_F(TDb, Serialize_DeSerialize_Cons) {
    auto serialization = theDb.Serialize();

    AdDb dbCopy(dbConfig, serialization);

    std::vector<std::shared_ptr<const FacebookAd>> matches;
    dbCopy.ForEachAdByConstituency([&](const auto& key, std::shared_ptr<const FacebookAd> ad) -> auto {
        matches.emplace_back(std::move(ad));
        return AdDb::DbScanOp::CONTINUE;
    });
    ASSERT_EQ(matches.size(), 3);

    AssertEq(*matches[0], ads[0]);
    AssertEq(*matches[1], ads[1]);
    AssertEq(*matches[2], ads[2]);
}

TEST_F(TDb, Serialize_DeSerialize_Issues) {
    auto serialization = theDb.Serialize();

    AdDb dbCopy(dbConfig, serialization);

    auto matches = dbCopy.GetIssue("Brexit");
    ASSERT_EQ(matches.size(), 1);

    AssertEq(*matches[0], ads[2]);
}

TEST_F(TDb, Serialize_DeSerialize_Issue_NewKey) {
    auto serialization = theDb.Serialize();

    pconfig.Get<Config::issues>()[0]->Get<Config::keys>().push_back("entity#0");
    AdDb dbCopy(pconfig.GetJSONString(), serialization, AdDb::DeSerialMode::FORCE_REINDEX);


    auto matches = dbCopy.GetIssue("Brexit");
    ASSERT_EQ(matches.size(), 2);

    AssertEq(*matches[0], ads[0]);
    AssertEq(*matches[1], ads[2]);
}

TEST_F(TDb, Serialize_DeSerialize_Cons_NewKey) {
    auto serialization = theDb.Serialize();

    pconfig.Get<Config::consituencies>()[0]->Get<Config::keys>().push_back("brexit");
    AdDb dbCopy(pconfig.GetJSONString(), serialization, AdDb::DeSerialMode::FORCE_REINDEX);


    auto matches = dbCopy.GetConstituency("Search#0");
    ASSERT_EQ(matches.size(), 2);

    AssertEq(*matches[0], ads[0]);
    AssertEq(*matches[1], ads[2]);
}

TEST_F(TDb, Serialize_DeSerialize_NewIssue) {
    auto serialization = theDb.Serialize();

    auto& issueConfig = *pconfig.Get<Config::issues>().emplace_back();
    issueConfig.Get<Config::id>() = "New Issue";
    issueConfig.Get<Config::keys>().push_back("entity#0");

    AdDb dbCopy(pconfig.GetJSONString(), serialization, AdDb::DeSerialMode::FORCE_REINDEX);


    auto matches = dbCopy.GetIssue("Brexit");
    ASSERT_EQ(matches.size(), 1);
    AssertEq(*matches[0], ads[2]);

    matches = dbCopy.GetIssue("New Issue");
    ASSERT_EQ(matches.size(), 1);
    AssertEq(*matches[0], ads[0]);
}

TEST_F(TDb, Serialize_DeSerialize_NewCon) {
    auto serialization = theDb.Serialize();

    auto& issueConfig = *pconfig.Get<Config::consituencies>().emplace_back();
    issueConfig.Get<Config::id>() = "Brexit";
    issueConfig.Get<Config::keys>().push_back("brexit");

    AdDb dbCopy(pconfig.GetJSONString(), serialization, AdDb::DeSerialMode::FORCE_REINDEX);


    auto matches = dbCopy.GetConstituency("Brexit");
    ASSERT_EQ(matches.size(), 1);
    AssertEq(*matches[0], ads[2]);

    matches = dbCopy.GetConstituency("Search#0");
    ASSERT_EQ(matches.size(), 1);
    AssertEq(*matches[0], ads[0]);

}

TEST_F(TDb, InvalidSerialization) {
    ASSERT_THROW((AdDb{dbConfig, {"}"}}), AdDb::InvalidSerializationError);
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

    std::sort(results.begin(), results.end(), [&] (const auto& lhs, const auto& rhs) -> bool {
        return (lhs->fundingEntity < rhs->fundingEntity);
    });

    const auto& libDemAd = *results[1];
    const auto& conAd = *results[0];

    ASSERT_EQ(libDemAd.fundingEntity, "Woking Liberal Democrats");
    ASSERT_EQ(conAd.fundingEntity, "Woking Conservative Association");
}

TEST(DbUtilsTest, RestoreWokingData) {
    auto oldDb = DbUtils::LoadDb("../test/data/cfg/woking.json", "../test/data/full_day_run");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 5;
    ad->fundingEntity = "Woking Test Org";
    ad->impressions.lower_bound = 123;
    oldDb->Store(std::move(ad));

    const auto DoCheck = [&] (AdDb& dbToTest, size_t libDemLower, size_t conLower) -> void {
        auto results = dbToTest.GetConstituency("Woking");
        ASSERT_EQ(results.size(), 3);

        std::sort(results.begin(), results.end(), [&] (const auto& lhs, const auto& rhs) -> bool {
            return (lhs->fundingEntity < rhs->fundingEntity);
        });

        const auto* libDemAd = results[1].get();
        const auto* conAd = results[0].get();
        const auto* testAd = results[2].get();

        ASSERT_EQ(libDemAd->fundingEntity, "Woking Liberal Democrats");
        ASSERT_EQ(libDemAd->impressions.lower_bound, libDemLower);
        ASSERT_EQ(libDemAd->id, 1572697905);
        ASSERT_EQ(conAd->fundingEntity, "Woking Conservative Association");
        ASSERT_EQ(conAd->impressions.lower_bound, conLower);
        ASSERT_EQ(conAd->id, 1572614327);
        ASSERT_EQ(testAd->fundingEntity, "Woking Test Org");
        ASSERT_EQ(testAd->impressions.lower_bound, 123);

    };
    ASSERT_NO_FATAL_FAILURE(DoCheck(*oldDb, 3000, 4000));

    DbUtils::WriteDbToDisk(*oldDb, "tmp.json");

    auto db = DbUtils::LoadDb("../test/data/cfg/woking.json", "", "tmp.json");

    ASSERT_NO_FATAL_FAILURE(DoCheck(*db, 3000, 4000));
}

TEST(DbUtilsTest, RestoreWokingData_Reindex) {
    auto oldDb = DbUtils::LoadDb("../test/data/cfg/woking.json", "../test/data/full_day_run");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 2572697905;
    ad->fundingEntity = "Woking Test Org";
    ad->impressions.lower_bound = 123;
    oldDb->Store(std::move(ad));

    const auto DoCheck = [&] (const std::string& conn, AdDb& dbToTest, size_t libDemLower, size_t conLower) -> void {
        auto results = dbToTest.GetConstituency(conn);
        ASSERT_EQ(results.size(), 3);

        std::sort(results.begin(), results.end(), [&] (const auto& lhs, const auto& rhs) -> bool {
            return (lhs->fundingEntity < rhs->fundingEntity);
        });

        const auto* libDemAd = results[1].get();
        const auto* conAd = results[0].get();
        const auto* testAd = results[2].get();

        ASSERT_EQ(libDemAd->fundingEntity, "Woking Liberal Democrats");
        ASSERT_EQ(libDemAd->impressions.lower_bound, libDemLower);
        ASSERT_EQ(libDemAd->id, 1572697905);
        ASSERT_EQ(conAd->fundingEntity, "Woking Conservative Association");
        ASSERT_EQ(conAd->impressions.lower_bound, conLower);
        ASSERT_EQ(conAd->id, 1572614327);
        ASSERT_EQ(testAd->fundingEntity, "Woking Test Org");
        ASSERT_EQ(testAd->impressions.lower_bound, 123);

    };
    DbUtils::WriteDbToDisk(*oldDb, "tmp.json");

    auto db = DbUtils::LoadDb("../test/data/cfg/testWoking.json", "", "tmp.json", AdDb::DeSerialMode::FORCE_REINDEX);

    ASSERT_NO_FATAL_FAILURE(DoCheck("testWoking", *db, 3000, 4000));
}

TEST(DbUtilsTest, RestoreAndUpdateWokingData) {
    auto oldDb = DbUtils::LoadDb("../test/data/cfg/woking.json", "../test/data/full_day_run");

    auto ad = std::make_unique<FacebookAd>();
    ad->id = 5;
    ad->fundingEntity = "Woking Test Org";
    ad->impressions.lower_bound = 123;
    oldDb->Store(std::move(ad));

    const auto DoCheck = [&] (AdDb& dbToTest, size_t libDemLower, size_t conLower) -> void {
        auto results = dbToTest.GetConstituency("Woking");
        ASSERT_EQ(results.size(), 3);

        std::sort(results.begin(), results.end(), [&] (const auto& lhs, const auto& rhs) -> bool {
            return (lhs->fundingEntity < rhs->fundingEntity);
        });

        const auto* libDemAd = results[1].get();
        const auto* conAd = results[0].get();
        const auto* testAd = results[2].get();

        ASSERT_EQ(libDemAd->fundingEntity, "Woking Liberal Democrats");
        ASSERT_EQ(libDemAd->impressions.lower_bound, libDemLower);
        ASSERT_EQ(libDemAd->id, 1572697905);
        ASSERT_EQ(conAd->fundingEntity, "Woking Conservative Association");
        ASSERT_EQ(conAd->impressions.lower_bound, conLower);
        ASSERT_EQ(conAd->id, 1572614327);
        ASSERT_EQ(testAd->fundingEntity, "Woking Test Org");
        ASSERT_EQ(testAd->impressions.lower_bound, 123);

    };
    ASSERT_NO_FATAL_FAILURE(DoCheck(*oldDb, 3000, 4000));

    DbUtils::WriteDbToDisk(*oldDb, "tmp.json");

    auto db = DbUtils::LoadDb("../test/data/cfg/woking.json", "../test/data/test_run", "tmp.json");

    ASSERT_NO_FATAL_FAILURE(DoCheck(*db, 3001, 4001));
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

class BreakDownTst: public TDb {};

TEST_F(BreakDownTst, BreakdownReport_Funder) {
    Reports::BreakdownReport rpt;
    rpt.keys = {"TestFunder1"};
    rpt.issueViews = {{}, {}};
    rpt.issueSpend = { {}, {} };
    rpt.consViews = { {}, {} };
    rpt.consSpend = { {}, {} };
    rpt.pageViews = { {}, {} };
    rpt.pageSpend = { {}, {} };

    DbUtils::WriteBreakdown(rpt, "testBreakdown.json");

    std::ifstream breakdownOutput("./testBreakdown.json");
    ASSERT_FALSE(breakdownOutput.fail());
    TestBrkDwn::BrkDwn outputParser;
    std::string rawSummary((std::istreambuf_iterator<char>(breakdownOutput)), std::istreambuf_iterator<char>());
    std::string error;
    ASSERT_TRUE(outputParser.Parse(rawSummary.c_str(), error)) << rawSummary << std::endl << error;
    ASSERT_EQ(outputParser.Get<TestBrkDwn::funders>().size(), 1);
    ASSERT_EQ(outputParser.Get<TestBrkDwn::funders>()[0], "TestFunder1");
}

TEST_F(BreakDownTst, BreakdownReport_Funders) {
    Reports::BreakdownReport rpt;
    rpt.keys = {"TestFunder1", "TestFunder2"};
    rpt.issueViews = {{}, {}};
    rpt.issueSpend = { {}, {} };
    rpt.consViews = { {}, {} };
    rpt.consSpend = { {}, {} };
    rpt.pageViews = { {}, {} };
    rpt.pageSpend = { {}, {} };

    DbUtils::WriteBreakdown(rpt, "testBreakdown.json");


    std::ifstream breakdownOutput("./testBreakdown.json");
    ASSERT_FALSE(breakdownOutput.fail());
    TestBrkDwn::BrkDwn outputParser;
    std::string rawSummary((std::istreambuf_iterator<char>(breakdownOutput)), std::istreambuf_iterator<char>());
    std::string error;
    ASSERT_TRUE(outputParser.Parse(rawSummary.c_str(), error)) << rawSummary << std::endl << error;
    ASSERT_EQ(outputParser.Get<TestBrkDwn::funders>().size(), 2);
    ASSERT_EQ(outputParser.Get<TestBrkDwn::funders>()[0], "TestFunder1");
    ASSERT_EQ(outputParser.Get<TestBrkDwn::funders>()[1], "TestFunder2");
}

using VennDataSet = std::vector<Reports::VennSet>;
template <class ReportDataSet, class ReportDataSubSet>
void TestVenn(const std::function<VennDataSet& (Reports::BreakdownReport& rpt)>& GetField) {
    Reports::BreakdownReport rpt;
    rpt.keys = {"TestFunder1", "TestFunder2"};
    rpt.issueViews = { {}, {} };
    rpt.issueSpend = { {}, {} };
    rpt.consViews = { {}, {} };
    rpt.consSpend = { {}, {} };
    rpt.pageViews = { {}, {} };
    rpt.pageSpend = { {}, {} };

    GetField(rpt) = {
            {
                    { "Group 1", "Group 2"},
                    {
                            { { 0}, 12},
                            { { 1}, 15},
                            { { 0, 1}, 2}
                    }
            }, {
                    { "Group 3"},
                    {
                            { { 0}, 13}
                    }
            }
    };


    DbUtils::WriteBreakdown(rpt, "testBreakdown.json");

    std::ifstream breakdownOutput("./testBreakdown.json");
    ASSERT_FALSE(breakdownOutput.fail());
    TestBrkDwn::BrkDwn outputParser;
    std::string rawSummary((std::istreambuf_iterator<char>(breakdownOutput)), std::istreambuf_iterator<char>());
    std::string error;
    ASSERT_TRUE(outputParser.Parse(rawSummary.c_str(), error)) << rawSummary << std::endl << error;

    auto& funder1 =
            outputParser.Get<ReportDataSet>()
                    .template Get<ReportDataSubSet>()
                    .template Get<TestBrkDwn::Venn::TestFunder1>();

    ASSERT_EQ(funder1.template Get<TestBrkDwn::name>(), "TestFunder1");
    ASSERT_EQ(funder1.template Get<TestBrkDwn::type>(), "venn");
    ASSERT_EQ(funder1.template Get<TestBrkDwn::Venn::data>().size(), 3);


    auto& funder2 =
            outputParser.Get<ReportDataSet>()
                    .template Get<ReportDataSubSet>()
                    .template Get<TestBrkDwn::Venn::TestFunder2>();

    ASSERT_EQ(funder2.template Get<TestBrkDwn::name>(), "TestFunder2");
    ASSERT_EQ(funder2.template Get<TestBrkDwn::type>(), "venn");
    ASSERT_EQ(funder2.template Get<TestBrkDwn::Venn::data>().size(), 1);

    const std::vector<size_t> values = {12, 15, 2};
    const std::vector<std::vector<std::string>> sets = {
            { "Group 1"},
            { "Group 2"},
            { "Group 1", "Group 2"}
    };


    for (size_t i = 0; i < 3; ++i) {
        const auto actValue = funder1.template Get<TestBrkDwn::Venn::data>()[i]->template Get<TestBrkDwn::value>();
        const auto& expValue = values[i];

        const std::vector<std::string>& expSet = sets[i];
        const std::vector<std::string>& actSet = funder1.template Get<TestBrkDwn::Venn::data>()[i]->template Get<TestBrkDwn::sets>();

        ASSERT_EQ(actValue, expValue) << "Failed matching Test Funder 1 value: " << i;

        ASSERT_EQ(expSet.size(), actSet.size());
        for (size_t j = 0; i < expSet.size(); ++i) {
            ASSERT_EQ(expSet[j], actSet[j]);
        }
    };

    const std::vector<size_t> values2 = {13};
    const std::vector<std::vector<std::string>> sets2 = {
            { "Group 3"}
    };

    for (size_t i = 0; i < 1; ++i) {
        const auto actValue = funder2.template Get<TestBrkDwn::Venn::data>()[i]->template Get<TestBrkDwn::value>();
        const auto& expValue = values2[i];

        const std::vector<std::string>& expSet = sets2[i];
        const std::vector<std::string>& actSet = funder2.template Get<TestBrkDwn::Venn::data>()[i]->template Get<TestBrkDwn::sets>();

        ASSERT_EQ(actValue, expValue) << "Failed matching Test Funder 2 value: " << i;

        ASSERT_EQ(expSet.size(), actSet.size());
        for (size_t j = 0; i < expSet.size(); ++i) {
            ASSERT_EQ(expSet[j], actSet[j]);
        }
    };

}

TEST_F(BreakDownTst, BreakdownReport_IssueViews) {
    const auto Getter = [] (Reports::BreakdownReport& rpt) -> VennDataSet& {
        return rpt.issueViews;
    };
    TestVenn<TestBrkDwn::Issues, TestBrkDwn::Venn::impressions>(Getter);
}

TEST_F(BreakDownTst, BreakdownReport_IssueSpend) {
    const auto Getter = [] (Reports::BreakdownReport& rpt) -> VennDataSet& {
        return rpt.issueSpend;
    };
    TestVenn<TestBrkDwn::Issues, TestBrkDwn::Venn::spend>(Getter);
}

TEST_F(BreakDownTst, BreakdownReport_ConsViews) {
    const auto Getter = [] (Reports::BreakdownReport& rpt) -> VennDataSet& {
        return rpt.consViews;
    };
    TestVenn<TestBrkDwn::Cons, TestBrkDwn::Venn::impressions>(Getter);
}

TEST_F(BreakDownTst, BreakdownReport_ConsSpend) {
    const auto Getter = [] (Reports::BreakdownReport& rpt) -> VennDataSet& {
        return rpt.consSpend;
    };
    TestVenn<TestBrkDwn::Cons, TestBrkDwn::Venn::spend>(Getter);
}

using PieDataSet = std::vector<Reports::PieMap>;
template <class ReportDataSet, class ReportDataSubSet>
void TestPie(const std::function<PieDataSet& (Reports::BreakdownReport& rpt)>& GetField) {
    Reports::BreakdownReport rpt;
    rpt.keys = {"TestFunder1", "TestFunder2"};
    rpt.issueViews = { {}, {} };
    rpt.issueSpend = { {}, {} };
    rpt.consViews = { {}, {} };
    rpt.consSpend = { {}, {} };
    rpt.pageViews = { {}, {} };
    rpt.pageSpend = { {}, {} };

    GetField(rpt) = {
        { {"Page 1", 2}, {"Page 2", 3} },
        { {"Page 3", 30} }
    };

    DbUtils::WriteBreakdown(rpt, "testBreakdown.json");

    std::ifstream breakdownOutput("./testBreakdown.json");
    ASSERT_FALSE(breakdownOutput.fail());
    TestBrkDwn::BrkDwn outputParser;
    std::string rawSummary((std::istreambuf_iterator<char>(breakdownOutput)), std::istreambuf_iterator<char>());
    std::string error;
    ASSERT_TRUE(outputParser.Parse(rawSummary.c_str(), error)) << rawSummary << std::endl << error;

    auto& funder1 =
            outputParser.Get<ReportDataSet>()
                    .template Get<ReportDataSubSet>()
                    .template Get<TestBrkDwn::Pie::TestFunder1>();

    ASSERT_EQ(funder1.template Get<TestBrkDwn::name>(), "TestFunder1");
    ASSERT_EQ(funder1.template Get<TestBrkDwn::type>(), "pie");
    ASSERT_EQ(funder1.template Get<TestBrkDwn::Pie::data>().size(), 2);

    ASSERT_EQ(funder1.template Get<TestBrkDwn::Pie::data>()[0]->template Get<TestBrkDwn::y>(), 2);
    ASSERT_EQ(funder1.template Get<TestBrkDwn::Pie::data>()[1]->template Get<TestBrkDwn::y>(), 3);

    ASSERT_EQ(funder1.template Get<TestBrkDwn::Pie::data>()[0]->template Get<TestBrkDwn::name>(), "Page 1");
    ASSERT_EQ(funder1.template Get<TestBrkDwn::Pie::data>()[1]->template Get<TestBrkDwn::name>(), "Page 2");


    auto& funder2 =
            outputParser.Get<ReportDataSet>()
                    .template Get<ReportDataSubSet>()
                    .template Get<TestBrkDwn::Pie::TestFunder2>();

    ASSERT_EQ(funder2.template Get<TestBrkDwn::name>(), "TestFunder2");
    ASSERT_EQ(funder2.template Get<TestBrkDwn::type>(), "pie");
    ASSERT_EQ(funder2.template Get<TestBrkDwn::Pie::data>().size(), 1);

    ASSERT_EQ(funder2.template Get<TestBrkDwn::Pie::data>()[0]->template Get<TestBrkDwn::name>(), "Page 3");

}

TEST_F(BreakDownTst, BreakdownReport_PagesViews) {
    const auto Getter = [] (Reports::BreakdownReport& rpt) -> PieDataSet& {
        return rpt.pageViews;
    };
    TestPie<TestBrkDwn::Pages, TestBrkDwn::Pie::impressions>(Getter);
}

TEST_F(BreakDownTst, BreakdownReport_PagesSpend) {
    const auto Getter = [] (Reports::BreakdownReport& rpt) -> PieDataSet& {
        return rpt.pageSpend;
    };
    TestPie<TestBrkDwn::Pages, TestBrkDwn::Pie::spend>(Getter);
}
