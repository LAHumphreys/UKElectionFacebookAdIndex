#include <iostream>
#include <fstream>
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/TimeSeriesConfigJSON.h"
#include <Reports.h>
#include <iomanip>
#include <util_time.h>
#include <set>

namespace {
    void Usage() {
        std::cout << "Usage: generateTmeSeries <cfg> <startDb> <endDb> <outputFile>" << std::endl;
    }

}
int main(int argc, const char* argv[]) {
    if (argc < 5) {
        Usage();
        return 1;
    }
    const std::string dbCfgFile = argv[1];
    const std::string startDbStore = argv[2];
    const std::string endDbStore = argv[3];
    const std::string output = argv[4];

    auto startDb = DbUtils::LoadDb(dbCfgFile, "", startDbStore);
    auto endDb = DbUtils::LoadDb(dbCfgFile, "", endDbStore);

    auto startReport = Reports::DoConsituencyReport(*startDb);
    auto endReport = Reports::DoConsituencyReport(*endDb);
    std::vector<Reports::Report*> conReportRefs = {startReport.get(), endReport.get()};

    std::cout << "Generating Consituency Time Series report...";
    std::cout.flush();
    auto conReport = Reports::DoTimeSeries(conReportRefs, Reports::TimeSeriesMode::REMOVE_BASELINE);
    std::cout << "done" << std::endl;

    struct ConItem {
        ConItem (const size_t& t, Reports::TimeSeriesItem& i, std::string name)
           : totalViews(t), report(i), name(name) {}
        size_t totalViews;
        Reports::TimeSeriesItem& report;
        std::string name;
        bool operator<(const ConItem& other) const {
            return (totalViews < other.totalViews);
        }
    };
    std::set<ConItem> rankedCons;
    for (auto& pair: *conReport) {
        size_t total = pair.second.residualImpressions[0];
        for (auto& viewPair: pair.second.guestimatedImpressions) {
            total += viewPair.second[0];
        }
        rankedCons.emplace(total, pair.second, pair.first);
    }

    for (auto it = rankedCons.begin(); it != rankedCons.end(); ++it) {
        std::cout << it->name << ": " << it->totalViews << std::endl;
    }

    SimpleJSONPrettyBuilder reportBuilder;

    const auto AddObject = [&] (const std::string& name, std::function<void ()> inner) {
        reportBuilder.AddName(name);
        reportBuilder.StartAnonymousObject();
        inner();
        reportBuilder.EndObject();
    };

    const auto AddAnnonObject = [&] (std::function<void ()> inner) {
        reportBuilder.StartAnonymousObject();
        inner();
        reportBuilder.EndObject();
    };

    const auto AddArray = [&] (const std::string& name, std::function<void ()> inner) {
        reportBuilder.StartArray(name);
        inner();
        reportBuilder.EndArray();
    };
    AddObject("views", [&] () {
        for (auto it = rankedCons.begin(); it != rankedCons.end(); ++it) {
            AddArray(it->name, [&] () {
                AddAnnonObject([&] () {
                    reportBuilder.Add("name", it->name + " 2017 Result");
                    AddArray("data", [&] () {
                        auto& views = it->report.guestimatedImpressions;
                        for (auto vit = views.begin(); vit != views.end(); ++vit) {
                            AddAnnonObject([&] () {
                                reportBuilder.Add("name", vit->first);
                                reportBuilder.Add("y", vit->second[0]);
                            });
                        }
                        AddAnnonObject([&] () {
                            reportBuilder.Add("name", std::string("Others"));
                            reportBuilder.Add("y", it->report.residualImpressions[0]);
                        });
                    });
                });
            });
        }
    });
    AddArray("summary", [&] () {
        size_t rank = 0;
        for (auto it = rankedCons.rbegin(); it != rankedCons.rend(); ++it) {
            ++rank;
            AddAnnonObject([&] () {
                reportBuilder.Add("name", it->name);
                reportBuilder.Add("rank", rank);
                reportBuilder.Add("guestimateImpressions", it->totalViews);
            });
        }
    });
    std::fstream reportFile(output, std::ios_base::out);
    reportFile << reportBuilder.GetAndClear();
    reportFile.close();

    return 0;

}
