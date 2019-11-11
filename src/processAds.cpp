//
// Created by lukeh on 04/11/2019.
//
#include <iostream>
#include "../internal_includes/DbUtils.h"
#include <Reports.h>
#include <iomanip>
#include <util_time.h>

namespace {
    void Usage() {
        std::cout << "Usage: processAds [options] <cfg> <directory to read> <directory to write>" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "   --redacted         :  Remove ad 'creative' details, producing a report suitable for distribution" << std::endl;
        std::cout << "   --loadDb [dbfile]  :  Initialize from an existing database" << std::endl;
        std::cout << "   --secureDb [dbfile]:  Secure the index to disk" << std::endl;
    }
}


int main(int argc, const char* argv[]) {
    DbUtils::WriteMode writeMode = DbUtils::WriteMode::FULL;
    if (argc < 4) {
        Usage();
        return 1;
    }
    size_t startIdx = argc - 3;

    std::string startDb = "";
    std::string resultDb = "";

    for (size_t i = 1; i < startIdx; ++i) {
        const std::string arg = argv[i];
        if (arg == "--redacted") {
            std::cout << "--redacted: Cretive details will be removed" << std::endl;
            writeMode = DbUtils::WriteMode::REDACTED;
        } else if (arg == "--loadDb") {
            ++i;
            startDb = argv[i];
            std::cout << "--loadDb: Initial state will be loaded from " << startDb << std::endl;
        } else if (arg == "--secureDb") {
            ++i;
            resultDb = argv[i];
            std::cout << "--secureDb: Final state will be secured to " << resultDb << std::endl;
        } else {
            Usage();
            return 1;
        }

        if (i > startIdx) {
            Usage();
            return 1;
        }
    }

    std::string cfg = argv[startIdx];
    std::string dataDir = argv[startIdx+1];

    std::string reportDir = argv[startIdx+2];

    auto db = DbUtils::LoadDb(cfg, dataDir, startDb);

    auto report = Reports::DoConsituencyReport(*db);
    DbUtils::WriteReport(*report, reportDir + "/Cons", writeMode);

    report = Reports::DoIssueReport(*db);
    DbUtils::WriteReport(*report, reportDir + "/Issues", writeMode);

    if (resultDb != "") {
        std::cout << "Securing database to disk... ";
        nstimestamp::Time secureStart;
        DbUtils::WriteDbToDisk(*db, resultDb);

        nstimestamp::Time secureEnd;
        std::cout << "done [" <<  secureEnd.DiffUSecs(secureStart) / 1000 << "ms]" << std::endl;
    }


    return 0;
}

