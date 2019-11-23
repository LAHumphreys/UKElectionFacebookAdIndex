#include <iostream>
#include <fstream>
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/TimeSeriesConfigJSON.h"
#include <Reports.h>
#include <iomanip>
#include <util_time.h>

namespace {
    void Usage() {
        std::cout << "Usage; reindex <cfg> <dbToIndex> <outputdb>" << std::endl;
    }
}

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        Usage();
        return 1;
    }

    std::string cfgPath = argv[1];
    std::string inputPath = argv[2];
    std::string outputPath = argv[3];

    std::cout << "Loading database from disk... ";
    std::cout.flush();
    nstimestamp::Time reindexStart;

    auto db = DbUtils::LoadDb(cfgPath, "", inputPath, AdDb::DeSerialMode::FORCE_REINDEX);

    nstimestamp::Time reindexStop;
    std::cout << "done [" <<  reindexStop.DiffUSecs(reindexStart) / 1000 << "ms]" << std::endl;

    std::cout << "Securing database to disk... ";
    std::cout.flush();
    nstimestamp::Time secureStart;

    DbUtils::WriteDbToDisk(*db, outputPath);

    nstimestamp::Time secureEnd;
    std::cout << "done [" <<  secureEnd.DiffUSecs(secureStart) / 1000 << "ms]" << std::endl;

    return 0;
}
