#include <gtest/gtest.h>
#include "../internal_includes/StringSearch.h"
#include "../internal_includes/DbUtils.h"

std::ostream& operator<<(std::ostream& os, const ValidKeys::MatchState& st) {
    switch (st) {
    case ValidKeys::MatchState::NO_MATCH:
        os << "NO_MATCH";
        break;
    case ValidKeys::MatchState::SUB_MATCH:
        os << "SUB_MATCH";
        break;
    case ValidKeys::MatchState::EXACT_MATCH:
        os << "EXACT_MATCH";
        break;

    }
    return os;
}

TEST(TestSortedKeys, EmptyKeys) {
    std::vector<std::string> keys = {};
    ValidKeys search(keys);

    ASSERT_EQ(search.State(), ValidKeys::MatchState::NO_MATCH);
}

TEST(TestSortedKeys, EmptyKeys_Refine) {
    std::vector<std::string> keys = {};
    ValidKeys search(keys);

    ASSERT_EQ(search.State(), ValidKeys::MatchState::NO_MATCH);

    search.Refine('a');

    ASSERT_EQ(search.State(), ValidKeys::MatchState::NO_MATCH);
}

TEST(TestSortedKeys, SingleCharKeys_NoMatch) {
    std::vector<std::string> keys = {"a", "b", "c", "d", "e"};
    ValidKeys search(keys);

    ASSERT_EQ(search.State(), ValidKeys::MatchState::SUB_MATCH);

    search.Refine('z');

    ASSERT_EQ(search.State(), ValidKeys::MatchState::NO_MATCH);
}

TEST(TestSortedKeys, SingleCharKeys_ExactMatch) {
    std::vector<std::string> keys = {"a", "b", "c", "d", "e"};
    ValidKeys search(keys);

    ASSERT_EQ(search.State(), ValidKeys::MatchState::SUB_MATCH);

    search.Refine('b');

    ASSERT_EQ(search.State(), ValidKeys::MatchState::EXACT_MATCH);
}

TEST(TestSortedKeys, SingleCharKeys_PartialMatch) {
    std::vector<std::string> keys = {"a", "b", "bbb", "c", "d", "e"};
    ValidKeys search(keys);

    ASSERT_EQ(search.State(), ValidKeys::MatchState::SUB_MATCH);

    search.Refine('b');

    ASSERT_EQ(search.State(), ValidKeys::MatchState::EXACT_MATCH);

    search.Refine('b');

    ASSERT_EQ(search.State(), ValidKeys::MatchState::SUB_MATCH);

    search.Refine('b');

    ASSERT_EQ(search.State(), ValidKeys::MatchState::EXACT_MATCH);

    search.Refine('b');

    ASSERT_EQ(search.State(), ValidKeys::MatchState::NO_MATCH);
}

TEST(TestSearchOneOf, NoKeys) {
    ASSERT_FALSE(DbUtils::SearchForOneOf("apples", {}));
}
