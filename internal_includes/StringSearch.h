//
// Created by lukeh on 24/12/2019.
//

#ifndef ELECTIONDATAANAL_STRINGSEARCH_H
#define ELECTIONDATAANAL_STRINGSEARCH_H

#include <string>
#include <vector>

class ValidKeys {
public:
    using KeyList = std::vector<std::string>;
    enum class MatchState {
        NO_MATCH,
        SUB_MATCH,
        EXACT_MATCH
    };
    ValidKeys(const KeyList& k)
            : keys(k)
    {
        Reset();
    }

    void Reset() {
        prefixLen = 0;
        if (keys.empty()) {
            state = MatchState::NO_MATCH;
        } else {
            state = MatchState::SUB_MATCH;
        }

        firstKey = 0;
        lastKey = keys.size();
        if (lastKey > 0) {
            lastKey-=1;
        }
    }


    void Refine (const char& next) {
        ++prefixLen;
        if (state != MatchState::NO_MATCH) {
            const size_t prefixInd = (prefixLen - 1);
            while (lastKey > firstKey && keys[lastKey][prefixInd] > next) {
                --lastKey;
            }
            while (firstKey < lastKey && keys[firstKey][prefixInd] < next) {
                ++firstKey;
            }

            if (GetChar(firstKey, prefixInd) != next) {
                state = MatchState::NO_MATCH;
            } else if (firstKey < keys.size() && keys[firstKey].size() == prefixLen) {
                state = MatchState::EXACT_MATCH;
            } else {
                state = MatchState::SUB_MATCH;
            }
        }
    }

    const MatchState& State() const {
        return state;
    }

private:
    inline char GetChar(const size_t& keyIdx, const size_t& charIdx) {
        char c = '\0';
        if (keys.size() > keyIdx) {
            const std::string& key = keys[keyIdx];

            if (key.size() > charIdx) {
                c = key[charIdx];
            }
        }

        return c;
    }

    size_t prefixLen;
    const KeyList& keys;
    size_t  firstKey;
    size_t lastKey;
    MatchState state;
};

#endif
