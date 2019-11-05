//
// Created by lukeh on 04/11/2019.
//

#ifndef ELECTIONDATAANAL_CONFIGPURIFY_H
#define ELECTIONDATAANAL_CONFIGPURIFY_H
#include <string>
#include <set>
#include <map>
#include <vector>

namespace CfgPurify {
    using FilteredKeysLists = std::map<std::string, std::vector<std::string>>;
    using BanList = std::set<std::string>;

    struct BadFile: std::exception {
        const char* what() const noexcept override {
            return "Could not load the data dictionary";
        }
    };
    struct BadCfg: std::exception {
        std::string error;
        const char* what() const noexcept override {
            return error.c_str();
        }
    };

    /*
     * Load in a dictionary from file - simple .txt with one "word" per line
     */
    BanList LoadDictionary(const std::string& path);

    std::string RemoveDuplicates(const std::string& cfg, FilteredKeysLists& filteredKeys);

    std::string RemoveBannedKeys(
            const BanList& banList,
            const std::string& cfg,
            FilteredKeysLists& filteredKeys);

}
#endif //ELECTIONDATAANAL_CONFIGPURIFY_H
