
#include <iostream>
#include <fstream>
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/TimeSeriesConfigJSON.h"
#include <Reports.h>
#include <iomanip>
#include <util_time.h>

namespace {
    void Usage() {
        std::cout << "Usage: generateTmeSeries <cfg> <dbCfg> <outputDir>" << std::endl;
    }

    struct CfgItem {
        std::string label;
        std::string dbFile;
    };

    std::vector<CfgItem> LoadCfg(const std::string& cfgPath) {
        thread_local TimeSeriesCfg::Cfg parser;
        std::ifstream cfgFile(cfgPath);
        if (cfgFile.fail()) {
            std::cout << "Failed to load: " << cfgPath << std::endl;
            Usage();
            exit(1);
        }
        std::string cfg((std::istreambuf_iterator<char>(cfgFile)), std::istreambuf_iterator<char>());
        std::string error;
        parser.Clear();
        if (!parser.Parse(cfg.c_str(), error)) {
            std::cout << "Failed to parse config: " << error << std::endl;
            Usage();
            exit(1);
        }

        std::vector<CfgItem> result;
        result.resize(parser.Get<TimeSeriesCfg::data>().size());
        for (size_t i = 0; i < result.size(); ++i) {
            result[i].dbFile = parser.Get<TimeSeriesCfg::data>()[i]->Get<TimeSeriesCfg::file>();
            result[i].label = parser.Get<TimeSeriesCfg::data>()[i]->Get<TimeSeriesCfg::label>();
        }

        return result;
    }

}
int main(int argc, const char* argv[]) {
    if (argc < 4) {
        Usage();
        return 1;
    }
    auto cfg = LoadCfg(argv[1]);
    const std::string dbCfgFile = argv[2];
    const std::string output = argv[3];

    std::vector<std::unique_ptr<Reports::Report>> issueReports;
    std::vector<Reports::Report*> issueReportsRefs;

    std::vector<std::unique_ptr<Reports::Report>> conReports;
    std::vector<Reports::Report*> conReportRefs;

    std::vector<std::string> issueLabels;
    for (const auto& dbCfg: cfg) {
        std::cout << "Generating data point: " << dbCfg.label << "...";
        std::cout.flush();
        issueLabels.push_back(dbCfg.label);
        auto db = DbUtils::LoadDb(dbCfgFile, "", dbCfg.dbFile);
        issueReportsRefs.push_back(
            issueReports.emplace_back(Reports::DoIssueReport(*db)).get());
        conReportRefs.push_back(
                conReports.emplace_back(Reports::DoConsituencyReport(*db)).get());
        std::cout << "done" << std::endl;
    }

    std::cout << "Generating Issues Time Series report...";
    std::cout.flush();
    auto timeReport = Reports::DoTimeSeries(issueReportsRefs, Reports::TimeSeriesMode::REMOVE_BASELINE);
    std::cout << "done" << std::endl;

    std::cout << "Generating Consituency Time Series report...";
    std::cout.flush();
    auto conReport = Reports::DoTimeSeries(conReportRefs, Reports::TimeSeriesMode::REMOVE_BASELINE);
    std::cout << "done" << std::endl;


    std::cout << "Securing reports to disk...";
    std::cout.flush();
    issueLabels.erase(issueLabels.begin());
    DbUtils::WriteTimeSeries(*timeReport, issueLabels, output + "/Issues.json");
    DbUtils::WriteTimeSeries(*conReport, issueLabels, output + "/Cons.json");
    std::cout << "done" << std::endl;


    return 0;

}
