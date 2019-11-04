#include <FacebookParser.h>
#include "../internal_includes/test_utils.h"
#include <gtest/gtest.h>
#include <fstream>
#include <OSTools.h>

const std::string json = R"JSON(
{
  "data": [
    {
    },
    {
      "ad_creative_link_title": "We will stop Brexit, invest in public services like our NHS and take action to fight climate change",
      "ad_creative_link_caption": "wokinglibdems.org.uk",
      "ad_creative_link_description": "<TEST>",
      "ad_creative_body": "Local Councillor, former Mayor and life-long resident Will Forster is the Lib Dem candidate to be Woking's new MP!",
      "ad_creation_time": "2019-10-29T17:16:59+0000",
      "ad_delivery_start_time": "2019-10-29T18:00:56+0000",
      "ad_delivery_stop_time": "2019-11-01T16:00:56+0000",
      "impressions": {
        "lower_bound": "8000",
        "upper_bound": "8999"
      },
      "spend": {
        "lower_bound": "0",
        "upper_bound": "99"
      },
      "region_distribution": [
        {
          "percentage": "0.984687",
          "region": "England"
        },
        {
          "percentage": "0.010652",
          "region": "Wales"
        },
        {
          "percentage": "0.003995",
          "region": "Scotland"
        },
        {
          "percentage": "0.000666",
          "region": "Northern Ireland"
        }
      ],
      "demographic_distribution": [
        {
          "percentage": "0.159091",
          "age": "55-64",
          "gender": "male"
        },
        {
          "percentage": "0.045455",
          "age": "35-44",
          "gender": "male"
        },
        {
          "percentage": "0.068182",
          "age": "25-34",
          "gender": "male"
        },
        {
          "percentage": "0.159091",
          "age": "55-64",
          "gender": "female"
        },
        {
          "percentage": "0.136364",
          "age": "65+",
          "gender": "male"
        },
        {
          "percentage": "0.045455",
          "age": "45-54",
          "gender": "male"
        },
        {
          "percentage": "0.204545",
          "age": "65+",
          "gender": "female"
        },
        {
          "percentage": "0.045455",
          "age": "25-34",
          "gender": "female"
        },
        {
          "percentage": "0.022727",
          "age": "35-44",
          "gender": "female"
        },
        {
          "percentage": "0.090909",
          "age": "45-54",
          "gender": "female"
        },
        {
          "percentage": "0.022727",
          "age": "18-24",
          "gender": "female"
        }
      ],
      "currency": "GBP",
      "funding_entity": "Woking Liberal Democrats",
      "page_name": "> Woking Liberal Democrats"
    }, {
      "ad_creative_link_title": "We will stop Brexit, invest in public services like our NHS and take action to fight climate change",
      "ad_creative_link_caption": "wokinglibdems.org.uk",
      "ad_creative_link_description": "<TEST>",
      "ad_creative_body": "Local Councillor, former Mayor and life-long resident Will Forster is the Lib Dem candidate to be Woking's new MP!",
      "ad_creation_time": "2020-10-29T17:16:59+0000",
      "ad_delivery_start_time": "2020-10-29T18:00:56+0000",
      "ad_delivery_stop_time": "2020-11-01T16:00:56+0000",
      "impressions": {
        "lower_bound": "1000000"
      }
    }
  ],
  "paging": {
    "cursors": {
      "after": "c2NyYXBpbmdfY3Vyc29yOk1UVTNNak0zTWpBMU5qb3lOVFUyTkRBNU5EWXhNamN4T1RReQZDZD"
    },
    "next": "https://graph.facebook.com/v5.0/ads_archive?access_token=EAAi1Yrtc0qIBAJxNetTgTTBaZBa2Kgk1ovk4ZCEGbljZANF5kV9rHoBQnqUgA6uetPik2VtoRPNkfMyZAhqg7ZAXrEDOsYbJ9iIglDTnZBdWAxoavZCPKdZAUJiDMIJ4FKx9JYaRsMGq0uUimimDteRBZBTmVDZCJiLINgk0FnrSXD0ZA7pB809zsGZCqVXSIai6M6WadaiZBlasKnyreAKkJ0hJl&pretty=0&fields=ad_creative_link_title%2Cad_creative_link_description%2Cad_creative_link_caption%2Cad_creative_body%2Cad_creation_time%2Cad_delivery_start_time%2Cad_delivery_stop_time%2Cimpressions%2Cspend%2Ccurrency%2Cfunding_entity%2Cpage_name&search_terms=%27Forster%27&ad_active_status=ALL&ad_reached_countries=%5B%27GB%27%5D&impression_condition=HAS_IMPRESSIONS_LAST_7_DAYS&limit=10&after=c2NyYXBpbmdfY3Vyc29yOk1UVTNNak0zTWpBMU5qb3lOVFUyTkRBNU5EWXhNamN4T1RReQZDZD"
  }
}
)JSON";

class AdParserTest: public ::testing::Test {
protected:
    using Parser = FacebookAdParser;
    using ParseResult = FacebookAdParser::ParseResult ;
    void WithTheAd(const std::function<void (FacebookAd& ad)>& f);
    FacebookAdParser parser;

};

void AdParserTest::WithTheAd(const std::function<void(FacebookAd &ad)> &f) {
    std::vector<FacebookAd> ads;
    ASSERT_EQ(parser.ParseFacebookAdQuery(json.c_str(), ads), ParseResult::VALID);
    ASSERT_EQ(ads.size(), 3);
    f(ads[1]);
}

TEST_F(AdParserTest, NullJSON) {
    std::vector<FacebookAd> ads;
    ASSERT_EQ(parser.ParseFacebookAdQuery(nullptr, ads), ParseResult::INVALID_STRING);
}

TEST_F(AdParserTest, InvalidJSON) {
    std::vector<FacebookAd> ads;
    ASSERT_EQ(parser.ParseFacebookAdQuery("{", ads), ParseResult::PARSE_ERROR);
}

TEST_F(AdParserTest, ValidData) {
    std::vector<FacebookAd> ads;
    ASSERT_EQ(parser.ParseFacebookAdQuery(json.c_str(), ads), ParseResult::VALID);
    ASSERT_EQ(ads.size(), 3);
}

TEST_F(AdParserTest, LinkTitle) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_STREQ(ad.linkTitle.c_str(), "We will stop Brexit, invest in public services like our NHS and take action to fight climate change");
    });
}

TEST_F(AdParserTest, LinkCaption) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_STREQ(ad.linkCaption.c_str(), "wokinglibdems.org.uk");
    });
}

TEST_F(AdParserTest, LinkDescription) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_STREQ(ad.linkDescription.c_str(), "<TEST>");
    });
}

TEST_F(AdParserTest, Body) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_STREQ(ad.body.c_str(), "Local Councillor, former Mayor and life-long resident Will Forster is the Lib Dem candidate to be Woking's new MP!");
    });
}

TEST_F(AdParserTest, CreationTime) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_EQ(ad.creationTime.Year(), 2019);
        ASSERT_EQ(ad.creationTime.Month(), 10);
        ASSERT_EQ(ad.creationTime.MDay(), 29);
        ASSERT_EQ(ad.creationTime.Hour(), 17);
        ASSERT_EQ(ad.creationTime.Minute(), 16);
        ASSERT_EQ(ad.creationTime.Second(), 59);
    });
}

TEST_F(AdParserTest, StartTime) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_EQ(ad.deliveryStartTime.Year(), 2019);
        ASSERT_EQ(ad.deliveryStartTime.Month(), 10);
        ASSERT_EQ(ad.deliveryStartTime.MDay(), 29);
        ASSERT_EQ(ad.deliveryStartTime.Hour(), 18);
        ASSERT_EQ(ad.deliveryStartTime.Minute(), 00);
        ASSERT_EQ(ad.deliveryStartTime.Second(), 56);
    });
}

TEST_F(AdParserTest, StopTime) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_EQ(ad.deliveryEndTime.Year(), 2019);
        ASSERT_EQ(ad.deliveryEndTime.Month(), 11);
        ASSERT_EQ(ad.deliveryEndTime.MDay(), 1);
        ASSERT_EQ(ad.deliveryEndTime.Hour(), 16);
        ASSERT_EQ(ad.deliveryEndTime.Minute(), 00);
        ASSERT_EQ(ad.deliveryEndTime.Second(), 56);
    });
}

TEST_F(AdParserTest, Impressions) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_EQ(ad.impressions.lower_bound, 8000);
        ASSERT_EQ(ad.impressions.upper_bound, 8999);
    });
}

TEST_F(AdParserTest, Impressions_NoUpperBound) {
    std::vector<FacebookAd> ads;
    ASSERT_EQ(parser.ParseFacebookAdQuery(json.c_str(), ads), ParseResult::VALID);
    ASSERT_EQ(ads.size(), 3);

    ASSERT_EQ(ads[2].impressions.lower_bound, 1000000);
    ASSERT_EQ(ads[2].impressions.upper_bound, 1000002);
}

TEST_F(AdParserTest, Spend) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_EQ(ad.spend.lower_bound, 0);
        ASSERT_EQ(ad.spend.upper_bound, 99);

        ASSERT_STREQ(ad.currency.c_str(), "GBP");
    });
}

TEST_F(AdParserTest, FundingEntity) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_STREQ(ad.fundingEntity.c_str(), "Woking Liberal Democrats");
    });
}

TEST_F(AdParserTest, PageName) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_STREQ(ad.pageName.c_str(), "> Woking Liberal Democrats");
    });
}

TEST_F(AdParserTest, Region) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_EQ(ad.regionDist.size(), 4);
        ASSERT_FLOAT_EQ(ad.regionDist[RegionCode::ENGLAND], 0.984687);
        ASSERT_FLOAT_EQ(ad.regionDist[RegionCode::NORTHERN_IRELAND], 0.000666);
        ASSERT_FLOAT_EQ(ad.regionDist[RegionCode::SCOTLAND], 0.003995);
        ASSERT_FLOAT_EQ(ad.regionDist[RegionCode::WALES], 0.010652);
    });
}

TEST_F(AdParserTest, Demographic) {
    WithTheAd([&] (FacebookAd& ad) -> void {
        ASSERT_EQ(ad.demographicDist.size(), 11);

        auto val = ad.demographicDist[{GenderCode::FEMALE, AgeRangeCode::AGE_18_TO_24}];
        ASSERT_FLOAT_EQ(val , 0.022727);

        val = ad.demographicDist[{GenderCode::MALE, AgeRangeCode::AGE_25_TO_34}];
        ASSERT_FLOAT_EQ(val , 0.068182);

        val = ad.demographicDist[{GenderCode::MALE, AgeRangeCode::AGE_35_TO_44}];
        ASSERT_FLOAT_EQ(val , 0.045455);

        val = ad.demographicDist[{GenderCode::FEMALE, AgeRangeCode::AGE_45_TO_54}];
        ASSERT_FLOAT_EQ(val , 0.090909);

        val = ad.demographicDist[{GenderCode::MALE, AgeRangeCode::AGE_55_TO_64}];
        ASSERT_FLOAT_EQ(val , 0.159091);

        val = ad.demographicDist[{GenderCode::FEMALE, AgeRangeCode::AGE_55_TO_64}];
        ASSERT_FLOAT_EQ(val , 0.159091);

        val = ad.demographicDist[{GenderCode::MALE, AgeRangeCode::AGE_65_AND_OVER}];
        ASSERT_FLOAT_EQ(val , 0.136364);
    });
}

TEST_F(AdParserTest, LoadFile) {
    std::vector<FacebookAd> ads;
    std::ifstream wokingFile("../test/data/woking.json");
    ASSERT_EQ(parser.Parse(wokingFile, ads), FacebookAdParser::ParseResult::VALID);
    ASSERT_EQ(ads.size(), 2);

    const auto& libDemAd = ads[0];
    const auto& conAd = ads[1];

    ASSERT_EQ(libDemAd.fundingEntity, "Woking Liberal Democrats");
    ASSERT_EQ(conAd.fundingEntity, "Woking Conservative Association");
}

TEST_F(AdParserTest, LoadDirectory) {
    std::vector<FacebookAd> ads;
    auto files = OS::Glob("../test/data/test_run/*");
    for (const std::string& path: files) {
        std::ifstream file(path);
        ASSERT_EQ(parser.Parse(file, ads), FacebookAdParser::ParseResult::VALID);
    }
    ASSERT_EQ(ads.size(), 2);

    const auto& libDemAd = ads[0];
    const auto& conAd = ads[1];

    // Whilst sort order is LOCALE defined, if someone's system is so obtuse as to sort 2 before 1, they're
    // welcome to pull request a fix for this test only problem
    ASSERT_EQ(libDemAd.fundingEntity, "Woking Liberal Democrats");
    ASSERT_EQ(conAd.fundingEntity, "Woking Conservative Association");
}
