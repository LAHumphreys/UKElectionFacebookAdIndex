//
// Created by lukeh on 04/11/2019.
//
#include "../internal_includes/ConfigPurify.h"
#include "../internal_includes/ConfigParser.h"
#include "../internal_includes/DbUtils.h"
#include <fstream>

using namespace Config;
using namespace CfgPurify;

namespace {
    std::string ToUpper(const std::string& target) {
        std::string upper(target);
        thread_local auto& f = std::use_facet<std::ctype<char>>(std::locale());
        f.toupper(&upper[0], &upper[0] + upper.size());

        return upper;
    }

    /*
     * Remove the tail, if it as as the very end of source
     */
    std::string RemoveTrailing(const std::string& source, const std::string& tail) {
        std::string result;
        if (tail.size() > source.size()) {
            result = source;
        } else {
            const size_t tailStart = (source.size() - tail.size());
            if (tail == (source.c_str() + tailStart)) {
                result = source.substr(0, tailStart);
            } else {
                result = source;
            }
        }
        return result;
    }

    /*
     * Remove the head, if it as as the very beging of the source
     */
    std::string RemoveLeading(const std::string& source, const std::string& head) {
        std::string result;
        if (head.size() > source.size()) {
            result = source;
        } else {
            const std::string sourceHead = source.substr(0, head.size());
            if (sourceHead == head) {
                result = source.substr(head.size());
            } else {
                result = source;
            }
        }
        return result;
    }
}

std::string CfgPurify::RemoveBannedKeys(
        const BanList& banList,
        const std::string &cfg,
        FilteredKeysLists& filteredKeys)
{
    BanList upperBannedList;
    for (const std::string& ban: banList) {
        upperBannedList.insert(ToUpper(ban));
    }
    DbConfig parser;
    std::string error;
    if (!parser.Parse(cfg.c_str(), error)) {
        BadCfg err;
        err.error = std::move(error);
        throw err;
    }

    for (auto& pitem: parser.Get<consituencies>()) {
        auto& itemKeys = pitem->Get<keys>();
        auto& removedKeys = filteredKeys[pitem->Get<id>()];
        std::vector<std::string> allowedKeys;

        allowedKeys.reserve(itemKeys.size());
        for (std::string& k: itemKeys) {
            std::string upperK = ToUpper(k);
            auto it = upperBannedList.find(upperK);
            if (it == upperBannedList.end()) {
                it = upperBannedList.find(RemoveTrailing(upperK, ", THE"));
            }
            if (it == upperBannedList.end()) {
                it = upperBannedList.find(RemoveLeading(upperK, "THE "));
            }
            if (it != upperBannedList.end() && upperK != ToUpper(pitem->Get<id>())) {
                removedKeys.push_back(std::move(k));
            } else {
                allowedKeys.emplace_back(std::move(k));
            }
        }
        itemKeys = std::move(allowedKeys);
    }
    return parser.GetPrettyJSONString();
}

CfgPurify::BanList CfgPurify::LoadDictionary(const std::string &path) {
    BanList list;
    std::fstream dictFile(path);
    if (dictFile.fail()) {
        throw CfgPurify::BadFile{};
    }
    std::string line;
    while(std::getline(dictFile, line)) {
        list.emplace(std::move(line));
    }
    return list;
}

std::string CfgPurify::RemoveDuplicates(const std::string &cfg, FilteredKeysLists &filteredKeys) {
    std::map<std::string, std::set<std::string>> cfgKeys;
    DbConfig parser;
    std::string error;
    if (!parser.Parse(cfg.c_str(), error)) {
        BadCfg err;
        err.error = std::move(error);
        throw err;
    }

    for (auto& pitem: parser.Get<consituencies>()) {
        auto& itemKeys = pitem->Get<keys>();

        for (std::string& k: itemKeys) {
            cfgKeys[ToUpper(k)].insert(pitem->Get<id>());
        }
    }

    std::map<std::string, std::vector<std::string>> cleanKeys;
    for (auto& pair: cfgKeys) {
        if (pair.second.size() == 1) {
            cleanKeys[*pair.second.begin()].push_back(pair.first);
        } else {
            for (const std::string& purgedFrom: pair.second) {
                if (DbUtils::Search(ToUpper(purgedFrom), pair.first)) {
                    // We can't remove the consituency itself from the search list!
                    cleanKeys[purgedFrom].push_back(pair.first);
                } else {
                    filteredKeys[purgedFrom].push_back(pair.first);
                }
            }
        }
    }

    for (auto& pitem: parser.Get<consituencies>()) {
        auto& itemKeys = pitem->Get<keys>();
        itemKeys = std::move(cleanKeys[pitem->Get<id>()]);
    }

    return parser.GetPrettyJSONString();
}
