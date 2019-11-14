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
        ad.id = 0;
        ad.creationTime = nstimestamp::Time("2019-10-29T16:15:59+0000");
        ad.fundingEntity = "Entity#0";
        ad.spend.lower_bound = 0;
        ad.spend.upper_bound = 99;
        ad.impressions.lower_bound = 0;
        ad.impressions.upper_bound = 99;

        FacebookAd& ad1 = ads.emplace_back();
        ad1.id = 1;
        ad1.creationTime = nstimestamp::Time("2019-10-29T18:16:59+0000");
        ad1.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad1.fundingEntity = "Entity#1";
        ad1.spend.lower_bound = 100;
        ad1.spend.upper_bound = 199;
        ad1.impressions.lower_bound = 100;
        ad1.impressions.upper_bound = 199;

        FacebookAd& ad2 = ads.emplace_back();
        ad2.id = 2;
        ad2.creationTime = nstimestamp::Time("2019-10-29T18:17:59+0000");
        ad2.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad2.fundingEntity = "Entity##1";
        ad2.spend.lower_bound = 1000;
        ad2.spend.upper_bound = 1200;
        ad2.impressions.lower_bound = 1000;
        ad2.impressions.upper_bound = 1200;

        FacebookAd& ad3 = ads.emplace_back();
        ad3.id = 3;
        ad3.creationTime = nstimestamp::Time("2019-10-29T18:17:60+0000");
        ad3.deliveryEndTime = nstimestamp::Time("2020-10-29T18:18:60+0000");
        ad3.fundingEntity = "Entity#1";
        ad3.spend.lower_bound = 0;
        ad3.spend.upper_bound = 99;
        ad3.impressions.lower_bound = 0;
        ad3.impressions.upper_bound = 99;

        FacebookAd& ad4 = ads.emplace_back();
        ad4.id = 4;
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

class DeltaReportTest: public ::testing::Test {
public:
    DeltaReportTest() : startDb(dbConfig), endDb(dbConfig) {}
protected:
    enum class CATEGORY  {
        SEARCH_0,
        SEARCH_1,
        SEARCH_2,
    };

    std::string GetKey(const CATEGORY& cat) {
        switch (cat) {
            case CATEGORY::SEARCH_0:
                return "Search#0";
            case CATEGORY::SEARCH_1:
                return "Search#1";
            case CATEGORY::SEARCH_2:
                return "Search#2";
        }
        throw "Shouldn't be here";
    }
    using KeyType = size_t;
    struct AdDelta {
        BoundedQuantity startImpressions;
        BoundedQuantity endImpressions;

        BoundedQuantity startSpend;
        BoundedQuantity endSpend;
    };

    AdDelta NewFlatDelta(const BoundedQuantity& base, const size_t& spendDiff, const size_t& impressDiff) {
        AdDelta delta;
        delta.startSpend = base;
        delta.endSpend = delta.startSpend;
        delta.endSpend.lower_bound += spendDiff;
        delta.endSpend.upper_bound += spendDiff;

        delta.startImpressions = base;
        delta.endImpressions = delta.startImpressions;
        delta.endImpressions.lower_bound += impressDiff;
        delta.endImpressions.upper_bound += impressDiff;

        return delta;
    }

    KeyType NewAddDiff(const CATEGORY& cat, const AdDelta& diff) {
        FacebookAd ad;
        ad.id = nextId++;
        struct UnknownCat{ };
        switch (cat) {
            case CATEGORY::SEARCH_0:
                ad.pageName = "entity#0";
                break;
            case CATEGORY::SEARCH_1:
                ad.pageName = "entity#1";
                break;
            default:
                throw UnknownCat{};
        }

        auto startAd = std::make_unique<FacebookAd>(ad);
        startAd->spend = diff.startSpend;
        startAd->impressions = diff.startImpressions;
        startDb.Store(std::move(startAd));

        auto endAd = std::make_unique<FacebookAd>(ad);
        endAd->spend = diff.endSpend;
        endAd->impressions = diff.endImpressions;
        endDb.Store(std::move(endAd));

        return ad.id;
    }

    std::unique_ptr<Reports::Report> DoDiff() {
        return Reports::DoDiffReport(startDb, endDb);
    }

    struct AdCheck {
        KeyType  id;
        long     estSpend;
        long     estImpressions;
    };

    struct ToCheck {
        CATEGORY cat;
        Reports::SummaryItem summary;
        std::vector<AdCheck> ads;
    };
    std::vector<ToCheck> EmptyChecks() {
        std::vector<ToCheck> checks = {
                { CATEGORY::SEARCH_0, {GetKey(CATEGORY::SEARCH_0), 0, 0, 0}, {} },
                { CATEGORY::SEARCH_1, {GetKey(CATEGORY::SEARCH_1), 0, 0, 0}, {} },
                { CATEGORY::SEARCH_2, {GetKey(CATEGORY::SEARCH_2), 0, 0, 0}, {} }
        };

        return checks;
    }

    void DoChecks(const std::vector<ToCheck>& checks, Reports::Report& report) {
        ASSERT_EQ(report.size(), checks.size());

        for (const auto& check: checks) {
            const std::string key = GetKey(check.cat);
            ASSERT_EQ(report[key].summary.name, check.summary.name);
            ASSERT_EQ(report[key].summary.count, check.summary.count);
            ASSERT_EQ(report[key].summary.estSpend, check.summary.estSpend);
            ASSERT_EQ(report[key].summary.estImpressions, check.summary.estImpressions);

            auto& reportAds = report[key].ads;

            ASSERT_EQ(reportAds.size(), check.ads.size());
            for (size_t i = 0; i < check.ads.size(); ++i) {
                ASSERT_EQ(reportAds[i].ad->id, check.ads[i].id);
            }

        }
    }
private:

    KeyType nextId = 0;
    AdDb startDb;
    AdDb endDb;
};


TEST_F( DeltaReportTest, NoAds) {
    auto ptr = DoDiff();
    auto report = *ptr;

    ASSERT_EQ(report.size(), 3);

    auto checks = EmptyChecks();

    ASSERT_NO_FATAL_FAILURE(
        DoChecks(checks, report)
    );
}

TEST_F( DeltaReportTest, NoDiff) {
    auto checks = EmptyChecks();

    // 1 add, same in both
    auto delta = NewFlatDelta({100, 200}, 0, 0);
    auto key = NewAddDiff(CATEGORY::SEARCH_0, delta);
    checks[0].ads.push_back({key, 0, 0});

    auto ptr = DoDiff();
    auto report = *ptr;

    ASSERT_NO_FATAL_FAILURE(
            DoChecks(checks, report)
    );

}
