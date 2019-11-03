//
// Created by lukeh on 03/11/2019.
//
#include "../internal_includes/DbUtils.h"
#include <fstream>
#include <OSTools.h>
#include <FacebookParser.h>

using namespace DbUtils;

std::unique_ptr<AdDb> DbUtils::LoadDb(const std::string &cfgPath, const std::string &dataDir) {
    std::ifstream cfgFile(cfgPath);
    std::unique_ptr<AdDb> result;
    if (cfgFile.fail()) {
        throw DbUtils::NoSuchCfgFile{cfgPath};
    } else {
        std::string cfg((std::istreambuf_iterator<char>(cfgFile)), std::istreambuf_iterator<char>());
        result = std::make_unique<AdDb> (cfg);
    }

    FacebookAdParser parser;
    std::vector<FacebookAd> ads;
    auto files = OS::Glob(dataDir + "/*");
    for (const std::string& path: files) {
        std::ifstream file(path);
        if (parser.Parse(file, ads) != FacebookAdParser::ParseResult::VALID) {
            throw DbUtils::BadData{};
        }
    }

    if (ads.size() == 0) {
        throw DbUtils::NoData{};
    }
    for (auto& ad: ads) {
        result->Store(std::make_unique<FacebookAd>(std::move(ad)));
    }

    return result;
}
