//
// Created by lukeh on 03/11/2019.
//
#include "../internal_includes/test_utils.h"
#include <AdDb.h>
#include <Reports.h>

const std::string dbConfig = R"JSON(
{
  "consituencies": [
    {
         "id": "Search#0",
         "keys": ["entity#0"]
    }, {
         "id": "Search#1",
         "keys": ["entity#1"]
    }, {
         "id": "Search#2",
         "keys": ["entity#DOES_NOT_EXIST"]
    }],

  "consituencies": [
    {
         "id": "Search#1",
         "keys": ["entity##1"]
    }],

    "issues": [{
         "id": "Brexit",
         "keys": ["brexit", "EU", "European Union", "350 million", "350,000,000", "350000000"]
    }
    ]
}
)JSON";

class ReportTest: public ::testing::Test {
public:
    ReportTest()
            : theDb(dbConfig)
    {
        FacebookAd& ad = ads.emplace_back();
        ad.creationTime = nstimestamp::Time("2019-10-29T16:15:59+0000");
        ad.fundingEntity = "Entity#0";
        ad.spend.lower_bound = 0;
        ad.spend.upper_bound = 99;
        ad.impressions.lower_bound = 0;
        ad.impressions.upper_bound = 99;

        FacebookAd& ad1 = ads.emplace_back();
        ad1.creationTime = nstimestamp::Time("2019-10-29T18:16:59+0000");
        ad1.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad1.fundingEntity = "Entity#1";
        ad1.spend.lower_bound = 100;
        ad1.spend.upper_bound = 199;
        ad1.impressions.lower_bound = 100;
        ad1.impressions.upper_bound = 199;

        FacebookAd& ad2 = ads.emplace_back();
        ad2.creationTime = nstimestamp::Time("2019-10-29T18:17:59+0000");
        ad2.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad2.fundingEntity = "Entity##1";
        ad2.spend.lower_bound = 1000;
        ad2.spend.upper_bound = 1200;
        ad2.impressions.lower_bound = 1000;
        ad2.impressions.upper_bound = 1200;

        FacebookAd& ad3 = ads.emplace_back();
        ad3.creationTime = nstimestamp::Time("2019-10-29T18:17:60+0000");
        ad3.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad3.fundingEntity = "Entity#1";
        ad3.spend.lower_bound = 0;
        ad3.spend.upper_bound = 99;
        ad3.impressions.lower_bound = 0;
        ad3.impressions.upper_bound = 99;

        FacebookAd& ad4 = ads.emplace_back();
        ad4.creationTime = nstimestamp::Time("2019-10-29T18:18:60+0000");
        ad4.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad4.fundingEntity = "Brexit";
        ad4.spend.lower_bound = 0;
        ad4.spend.upper_bound = 99;
        ad4.impressions.lower_bound = 0;
        ad4.impressions.upper_bound = 99;
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

TEST_F(ReportTest, ConsituencyMentions_Summary) {
    auto report = *Reports::DoConsituencyReport(theDb);

    ASSERT_EQ(report.size(), 3);

    ASSERT_EQ(report["Search#0"].summary.name, "Search#0");
    ASSERT_EQ(report["Search#0"].summary.count, 1);
    ASSERT_EQ(report["Search#0"].summary.estImpressions, 1);
    ASSERT_EQ(report["Search#0"].summary.estSpend, 1);

    ASSERT_EQ(report["Search#1"].summary.name, "Search#1");
    ASSERT_EQ(report["Search#1"].summary.count, 3);
    ASSERT_EQ(report["Search#1"].summary.estImpressions, 1 + 149 + 1100);
    ASSERT_EQ(report["Search#1"].summary.estSpend, 0 + 149 + 1100);

    ASSERT_EQ(report["Search#2"].summary.name, "Search#2");
    ASSERT_EQ(report["Search#2"].summary.count, 0);
    ASSERT_EQ(report["Search#2"].summary.estImpressions, 0);
    ASSERT_EQ(report["Search#2"].summary.estSpend, 0);
}

TEST_F(ReportTest, ConsituencyMentions_AdList) {
    auto report = *Reports::DoConsituencyReport(theDb);

    ASSERT_EQ(report.size(), 3);

    ASSERT_EQ(report["Search#0"].ads.size(), 1);
    ASSERT_EQ(report["Search#0"].ads[0].ad->fundingEntity, "Entity#0");

    ASSERT_EQ(report["Search#1"].ads.size(), 3);
    ASSERT_EQ(report["Search#1"].ads[0].ad->fundingEntity, "Entity#1");
    ASSERT_EQ(report["Search#1"].ads[1].ad->fundingEntity, "Entity##1");
    ASSERT_EQ(report["Search#1"].ads[2].ad->fundingEntity, "Entity#1");

    ASSERT_EQ(report["Search#2"].ads.size(), 0);
}

TEST_F(ReportTest, ConsituencyMentions_AdList_Spend) {
    auto report = *Reports::DoConsituencyReport(theDb);

    ASSERT_EQ(report.size(), 3);

    ASSERT_EQ(report["Search#0"].ads.size(), 1);
    ASSERT_EQ(report["Search#0"].ads[0].guestimateSpend, 1);

    ASSERT_EQ(report["Search#1"].ads.size(), 3);
    ASSERT_EQ(report["Search#1"].ads[0].guestimateSpend, 149);
    ASSERT_EQ(report["Search#1"].ads[1].guestimateSpend, 1100);
    ASSERT_EQ(report["Search#1"].ads[2].guestimateSpend, 1);

    ASSERT_EQ(report["Search#2"].ads.size(), 0);
}

TEST_F(ReportTest, ConsituencyMentions_AdList_Impressions) {
    auto report = *Reports::DoConsituencyReport(theDb);

    ASSERT_EQ(report.size(), 3);

    ASSERT_EQ(report["Search#0"].ads.size(), 1);
    ASSERT_EQ(report["Search#0"].ads[0].guestimateImpressions, 1);

    ASSERT_EQ(report["Search#1"].ads.size(), 3);
    ASSERT_EQ(report["Search#1"].ads[0].guestimateImpressions, 149);
    ASSERT_EQ(report["Search#1"].ads[1].guestimateImpressions, 1100);
    ASSERT_EQ(report["Search#1"].ads[2].guestimateImpressions, 1);

    ASSERT_EQ(report["Search#2"].ads.size(), 0);
}

TEST_F(ReportTest, Issues_Summary) {
    auto report = *Reports::DoIssueReport(theDb);

    ASSERT_EQ(report.size(), 1);

    ASSERT_EQ(report["Brexit"].summary.name, "Brexit");
    ASSERT_EQ(report["Brexit"].summary.count, 1);
    ASSERT_EQ(report["Brexit"].summary.estImpressions, 1);
    ASSERT_EQ(report["Brexit"].summary.estSpend, 1);
}
