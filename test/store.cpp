//
// Created by lukeh on 01/11/2019.
//

#include "../internal_includes/test_utils.h"
#include <FacebookAdStore.h>
#include <gtest/gtest.h>
#include <util_time.h>

using namespace nstimestamp;

class TestFacebookStore: public ::testing::Test {
public:
    TestFacebookStore() {
        FacebookAd& ad = ads.emplace_back();
        ad.id = 0;
        ad.creationTime = nstimestamp::Time("2019-10-29T17:16:59+0000");
        ad.fundingEntity = "Entity#0";
        ad.pageName = "Page#0";
        ad.linkTitle = "Title#0";
        ad.linkCaption = "Caption#0";
        ad.linkDescription = "Description#0";
        ad.body = "Body#0";

        FacebookAd& ad1 = ads.emplace_back();
        ad1.id = 1;
        ad1.creationTime = nstimestamp::Time("2019-10-29T18:16:59+0000");
        ad1.fundingEntity = "Entity#1";
        ad1.pageName = "Page#1";
        ad1.linkTitle = "Title#1";
        ad1.linkCaption = "Caption#1";
        ad1.linkDescription = "Description#1";
        ad1.body = "Body#1";
    }
protected:
    std::vector<FacebookAd> ads;
    FacebookAdStore theStore;
};

TEST_F(TestFacebookStore, NoSuchItem) {
    ASSERT_TRUE(theStore.Get(99999).IsNull());
}

TEST_F(TestFacebookStore, InitialStore) {
    auto& ref = theStore.Store(std::make_unique<FacebookAd>(ads[0]));
    ASSERT_FALSE(ref.IsNull());

    AssertEq(ads[0], ref.ItemRef());
}

TEST_F(TestFacebookStore, RetrieveFromStore) {
    std::vector<StoredFacebookAd::KeyType> keys = {
        theStore.Store(std::make_unique<FacebookAd>(ads[0])).Key(),
        theStore.Store(std::make_unique<FacebookAd>(ads[1])).Key(),
    };
    for (size_t i = 0; i < keys.size(); ++i) {
        const auto& key = keys[i];
        const auto& ad = ads[i];
        auto& ref = theStore.Get(key);
        ASSERT_FALSE(ref.IsNull());
        AssertEq(ad, ref.ItemRef());
    }
}

TEST_F(TestFacebookStore, UpdateItem) {
    std::vector<StoredFacebookAd::KeyType> keys = {
            theStore.Store(std::make_unique<FacebookAd>(ads[0])).Key(),
            theStore.Store(std::make_unique<FacebookAd>(ads[1])).Key(),
    };

    std::vector<FacebookAd> updatedAds {
        ads[0],
        ads[1]
    };

    updatedAds[0].regionDist.try_emplace(RegionCode::ENGLAND, 1.0);
    updatedAds[1].spend.lower_bound = 3000;
    updatedAds[1].spend.upper_bound = 3999;

    theStore.Store(std::make_unique<FacebookAd>(updatedAds[0]));
    theStore.Store(std::make_unique<FacebookAd>(updatedAds[1]));


    for (size_t i = 0; i < keys.size(); ++i) {
        const auto& key = keys[i];
        const auto& ad = updatedAds[i];
        auto& ref = theStore.Get(key);
        ASSERT_FALSE(ref.IsNull());
        AssertEq(ad, ref.ItemRef());
    }

}

TEST_F(TestFacebookStore, PatchNoKeyChange) {
    StoredFacebookAd storedAd { std::make_shared<FacebookAd>(ads[0])};

    FacebookAd updatedAd(ads[0]);
    updatedAd.id = 77;

    // NOTE: This shouldn't actually happen without a code error in the actual store...
    ASSERT_THROW(storedAd.PatchStoredValues(std::move(updatedAd)), StoredFacebookAd::KeyChangeError);


}
