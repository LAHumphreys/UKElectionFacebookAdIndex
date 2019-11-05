#include <gtest/gtest.h>
#include <IndexConfig.h>
#include "FacebookAdKey.h"
#include <memory>
#include <Index.h>

#include "../internal_includes/test_utils.h"

class IndexTest: public ::testing::Test {
public:
    IndexTest() {
        constituencyConfig = std::make_shared<IndexConfig>(
         std::vector<IndexConfig::Item>{
            {
                "Woking",
                {"Woking", "Zoëy", "Forster"}
            }, {
                "Other",
                {"Other", "Corbyn"}
            }
        });

        index = std::make_unique<Index<FacebookAdKey>>(
                constituencyConfig,
                facebookKey);

    }
protected:
    std::shared_ptr<IndexConfig> constituencyConfig;
    std::shared_ptr<FacebookAdKey> facebookKey =
            std::make_shared<FacebookAdKey>();
    std::unique_ptr<Index<FacebookAdKey>> index;
};


TEST_F(IndexTest, NoEntries) {
    auto hits = index->Get("Not a con");
    ASSERT_EQ(hits.size(), 0);

    hits = index->Get(constituencyConfig->items[0].id);
    ASSERT_EQ(hits.size(), 0);
}

class IndexMatchTest: public IndexTest {
protected:
    std::vector<StoredFacebookAd::KeyType> expectedWokingKeys;

    std::function<void(std::string text, FacebookAd& ad)> Setter;

    void GenerateNonMatch(std::string text) {
        auto ad = std::make_shared<FacebookAd>();
        ad->creationTime.SetNow();
        Setter(std::move(text), *ad);

        auto nonMatch = StoredFacebookAd(std::move(ad));
        index->Update(std::move(nonMatch));
    }

    void GenerateMatch (std::string text) {
        auto ad = std::make_shared<FacebookAd>();
        ad->creationTime.SetNow();
        Setter(std::move(text), *ad);

        auto wokingUpdate = StoredFacebookAd(std::move(ad));
        expectedWokingKeys.push_back(wokingUpdate.Key());
        index->Update(std::move(wokingUpdate));
    };

    void DoWokingTest() {
        GenerateMatch("Everything's happening in Woking!");
        GenerateMatch("Everything's happening in wokIng!");
        GenerateMatch("Unicode check time: Zoëy");
        GenerateMatch("Woking at the start!");
        GenerateMatch("At the end: Woking");

        GenerateNonMatch("Not an interesting one at all!");

        GenerateNonMatch("Attempted trap: Wokingham");
        GenerateMatch("But don't throw the Wokingham baby out with the Woking bathwater");

        GenerateNonMatch("Attempted trap: prefixWoking");

        auto wokingHits = index->Get("Woking");
        ASSERT_EQ(expectedWokingKeys.size(), wokingHits.size());

        for (size_t i = 0; i < expectedWokingKeys.size(); ++i) {
            ASSERT_EQ(expectedWokingKeys[i], wokingHits[i]);
        }
    }

};

TEST_F(IndexMatchTest, MatchTitle) {
    Setter = [&] (std::string toSet, FacebookAd& ad) {
        ad.linkTitle = std::move(toSet);
    };

    DoWokingTest();
}

TEST_F(IndexMatchTest, MatchBody) {
    Setter = [&] (std::string toSet, FacebookAd& ad) {
        ad.body = std::move(toSet);
    };

    DoWokingTest();
}

TEST_F(IndexMatchTest, MatchDesscription) {
    Setter = [&] (std::string toSet, FacebookAd& ad) {
        ad.linkDescription = std::move(toSet);
    };

    DoWokingTest();
}

TEST_F(IndexMatchTest, MatchCaption) {
    Setter = [&] (std::string toSet, FacebookAd& ad) {
        ad.linkCaption = std::move(toSet);
    };

    DoWokingTest();
}

TEST_F(IndexMatchTest, Funder) {
    Setter = [&] (std::string toSet, FacebookAd& ad) {
        ad.fundingEntity = std::move(toSet);
    };

    DoWokingTest();
}

TEST_F(IndexMatchTest, PageName) {
    Setter = [&] (std::string toSet, FacebookAd& ad) {
        ad.pageName = std::move(toSet);
    };

    DoWokingTest();
}
