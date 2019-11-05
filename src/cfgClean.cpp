#include "../internal_includes/SummmaryJSON.h"
#include "../internal_includes/ConfigPurify.h"
#include <gtest/gtest.h>
#include <AdDb.h>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: cfgClean <cfgToClean> <engDictionary> <output>" << std::endl;
        return 1;
    }
    std::string cfgPath = argv[1];
    std::string dictionary = argv[2];
    std::string outPath = argv[3];
    auto banList = CfgPurify::LoadDictionary(dictionary);

    std::ifstream cfgFile(cfgPath);
    if (cfgFile.fail()) {
        std::cout << "No such file: " << cfgPath << std::endl;
        return 1;
    }
    std::ofstream cleanCfgFile(outPath);
    if (cleanCfgFile.fail()) {
        std::cout << "Could not open for write: " << cfgPath << std::endl;
        return 1;
    }

    std::string cfg((std::istreambuf_iterator<char>(cfgFile)), std::istreambuf_iterator<char>());

    CfgPurify::FilteredKeysLists removed;
    std::string cleanCfg = CfgPurify::RemoveBannedKeys(banList, cfg, removed);
    cleanCfg = CfgPurify::RemoveDuplicates(cleanCfg, removed);

    if (!removed.empty()) {
        std::cout << "Removed the following keys:" << std::endl;
        for (auto& p: removed) {
            if (!p.second.empty()) {
                const std::string& id = p.first;
                std::cout << "  " << id << std::endl;
                for (const std::string& key: p.second) {
                    std::cout << "      " << key << std::endl;
                }
            }
        }
    }

    cleanCfgFile << cleanCfg;

    return 0;
}
