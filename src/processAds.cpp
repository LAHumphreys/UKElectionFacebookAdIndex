//
// Created by lukeh on 04/11/2019.
//
#include <iostream>
#include "../internal_includes/DbUtils.h"
#include <Reports.h>
#include <iomanip>

namespace {
    void Usage() {
        std::cout << "Usage: processAds [options] <cfg> <directory to read> <directory to write>" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "   --redacted:  Remove ad 'creative' details, producing a report suitable for distribution" << std::endl;
    }
}


int main(int argc, const char* argv[]) {
    DbUtils::WriteMode writeMode = DbUtils::WriteMode::FULL;
    if (argc < 4) {
        Usage();
        return 1;
    }
    size_t startIdx = argc - 3;

    for (size_t i = 1; i < startIdx; ++i) {
        const std::string arg = argv[i];
        if (arg == "--redacted") {
            std::cout << "--redacted: Cretive details will be removed" << std::endl;
            writeMode = DbUtils::WriteMode::REDACTED;
        } else {
            Usage();
            return 1;
        }
    }

    std::string cfg = argv[startIdx];
    std::string dataDir = argv[startIdx+1];

    std::string reportDir = argv[startIdx+2];

    auto db = DbUtils::LoadDb(cfg, dataDir);

    auto report = Reports::DoConsituencyReport(*db);
    DbUtils::WriteReport(*report, reportDir + "/Cons", writeMode);
    std::cout << "Consituencies: " << std::endl;
    for (auto& pair: *report) {
        std::cout << "    " << std::setw(50) << pair.first << ": " << pair.second.summary.count << std::endl;
    }

    report = Reports::DoIssueReport(*db);
    DbUtils::WriteReport(*report, reportDir + "/Issues", writeMode);
    std::cout << "Issues: " << std::endl;
    for (auto& pair: *report) {
        std::cout << "    " << std::setw(50) << pair.first << ": " << pair.second.summary.count << std::endl;
    }


    return 0;
}

