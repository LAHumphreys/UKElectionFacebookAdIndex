//
// Created by lukeh on 04/11/2019.
//
#include <iostream>
#include "../internal_includes/DbUtils.h"
#include <Reports.h>
#include <iomanip>

int main(int argc, const char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: processAds <cfg> <directory to read> <directory to write>" << std::endl;
        return 1;
    }
    std::string cfg = argv[1];
    std::string dataDir = argv[2];

    std::string reportDir = argv[3];

    auto db = DbUtils::LoadDb(cfg, dataDir);

    auto report = Reports::DoConsituencyReport(*db);
    DbUtils::WriteReport(*report, reportDir + "/Cons");
    std::cout << "Consituencies: " << std::endl;
    for (auto& pair: *report) {
        std::cout << "    " << std::setw(50) << pair.first << ": " << pair.second.summary.count << std::endl;
    }

    report = Reports::DoIssueReport(*db);
    DbUtils::WriteReport(*report, reportDir + "/Issues");
    std::cout << "Issues: " << std::endl;
    for (auto& pair: *report) {
        std::cout << "    " << std::setw(50) << pair.first << ": " << pair.second.summary.count << std::endl;
    }


    return 0;
}

