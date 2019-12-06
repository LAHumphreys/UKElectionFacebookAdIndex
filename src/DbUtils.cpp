//
// Created by lukeh on 03/11/2019.
//
#include "../internal_includes/DbUtils.h"
#include "../internal_includes/SummmaryJSON.h"
#include <fstream>
#include <OSTools.h>
#include <FacebookParser.h>
#include <Reports.h>
#include <logger.h>

using namespace DbUtils;
using namespace nstimestamp;

namespace {
    constexpr bool IsWordChar(char c) {
        if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            return true;
        } else {
            return false;
        }
    }
}
bool DbUtils::Search(const std::string& toSearch, const std::string& key) {
    size_t pos = 0;
    bool found = false;
    while (!found && pos != std::string::npos) {
        pos = toSearch.find(key, pos);

        bool match = true;
        const size_t next = pos + key.size();

        if (pos == std::string::npos) {
            match = false;
        }
        if (match && next < toSearch.size()) {
            char nchar = toSearch[next];
            match = !IsWordChar(nchar);
        }

        if (match && pos > 0) {
            char pchar = toSearch[pos-1];
            match = !IsWordChar(pchar);
        }

        if (match) {
            found = true;
        } else if (pos != std::string::npos) {
            ++pos;
        }
    }

    return found;
}

std::unique_ptr<AdDb>
DbUtils::LoadDb(
        const std::string &cfgPath,
        const std::string &dataDir,
        const std::string &dbStartState,
        const AdDb::DeSerialMode& indexMode)
{
    Time start;
    std::ifstream cfgFile(cfgPath);
    std::unique_ptr<AdDb> result;
    if (cfgFile.fail()) {
        throw DbUtils::NoSuchCfgFile{cfgPath};
    } else {
        std::string cfg((std::istreambuf_iterator<char>(cfgFile)), std::istreambuf_iterator<char>());
        if (dbStartState != "") {
            std::ifstream dbFile(dbStartState);
            std::string dbData((std::istreambuf_iterator<char>(dbFile)), std::istreambuf_iterator<char>());
            AdDb::Serialization data;
            data.json = std::move(dbData);
            result = std::make_unique<AdDb> (cfg, data, indexMode);
        } else {
            result = std::make_unique<AdDb> (cfg);
        }
    }

    if (dataDir != "") {
        FacebookAdParser parser;
        std::vector<std::unique_ptr<FacebookAd>> ads;
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

        Time loadEnd;
        SLOG_FROM(LOG_OVERVIEW, __func__,
                  "Loaded " << ads.size() << " ads in " << loadEnd.DiffSecs(start) << "s")

        SLOG_FROM(LOG_OVERVIEW, __func__, "Building indexes...");

        size_t i = 0;
        Time lastReport;
        for (auto& ad: ads) {
            result->Store(std::move(ad));

            if (i %10 == 0) {
                Time now;
                if (now.DiffSecs(lastReport) >= 10) {
                    SLOG_FROM(LOG_OVERVIEW, __func__,
                              "Indexed " << i << " of " << ads.size() << " ads in " << now.DiffSecs(loadEnd) << "s")
                    lastReport = now;
                }
            }

            ++i;
        }

        Time storeEnd;
        SLOG_FROM(LOG_OVERVIEW, __func__,
                  "Indexed " << ads.size() << " ads in " << storeEnd.DiffSecs(loadEnd) << "s")

    }
    return result;
}

std::unique_ptr<AdDb> DbUtils::LoadDb(const std::string &cfgPath, const std::string &dataDir) {
    return LoadDb(cfgPath, dataDir, "");
}

void DbUtils::WriteTimeSeries(Reports::TimeSeriesReport &report, const std::vector<std::string> timeStamps, const std::string &path) {
    std::fstream reportFile(path, std::ios_base::out);
    std::vector<std::string> catts;
    catts.reserve(report.size());
    for (const auto& pair: report) {
        catts.push_back(pair.first);
    }

    SimpleJSONPrettyBuilder reportBuilder;
    reportBuilder.Add("cattegories", catts);
    reportBuilder.Add("timeSeries", timeStamps);

    for (const std::string& catt: catts) {
        reportBuilder.AddName(catt);
        reportBuilder.StartAnonymousObject();
            reportBuilder.StartArray("spend");
                reportBuilder.StartAnonymousObject();
                    reportBuilder.Add("name", std::string("Other"));
                    reportBuilder.Add("data", report[catt].residualSpend);
                reportBuilder.EndObject();
                for (auto& pair: report[catt].guestimatedSpend) {
                    reportBuilder.StartAnonymousObject();
                        reportBuilder.Add("name", pair.first);
                        reportBuilder.Add("data", pair.second);
                    reportBuilder.EndObject();
                }
            reportBuilder.EndArray();

            reportBuilder.StartArray("impressions");
                reportBuilder.StartAnonymousObject();
                    reportBuilder.Add("name", std::string("Other"));
                    reportBuilder.Add("data", report[catt].residualImpressions);
                reportBuilder.EndObject();
                for (auto& pair: report[catt].guestimatedImpressions) {
                    reportBuilder.StartAnonymousObject();
                        reportBuilder.Add("name", pair.first);
                        reportBuilder.Add("data", pair.second);
                    reportBuilder.EndObject();

                }
            reportBuilder.EndArray();
        reportBuilder.EndObject();
    }

    reportFile << reportBuilder.GetAndClear();
}

void DbUtils::WriteReport(Reports::Report& report, const std::string &basePath, WriteMode mode) {
    std::fstream summaryFile(basePath + "/Summary.json", std::ios_base::out);
    SimpleJSONPrettyBuilder summaryBuilder;
    summaryBuilder.StartArray("summary");
    for (const auto &item: report) {
        summaryBuilder.StartAnonymousObject();
        summaryBuilder.Add("name", item.first);
        summaryBuilder.Add("totalAds", item.second.summary.count);
        summaryBuilder.Add("guestimateImpressions", item.second.summary.estImpressions);
        summaryBuilder.Add("guestimateSpendGBP", item.second.summary.estSpend);
        summaryBuilder.EndObject();

        std::fstream adsFile(basePath + "/" + item.first + ".json", std::ios_base::out);
        SimpleJSONPrettyBuilder adsBuilder;
        adsBuilder.StartArray("data");
        for (const auto& ad: item.second.ads) {
            adsBuilder.StartAnonymousObject();
            adsBuilder.Add("funding_entity", ad.ad->fundingEntity);
            adsBuilder.Add("ad_delivery_start_time", ad.ad->deliveryStartTime.ISO8601Timestamp());
            adsBuilder.Add("ad_delivery_end_time", ad.ad->deliveryEndTime.ISO8601Timestamp());
            adsBuilder.Add("ad_creation_time", ad.ad->creationTime.ISO8601Timestamp());
            if (mode == WriteMode::REDACTED) {
                const std::string redacted(REDACTED_TEXT);
                adsBuilder.Add("ad_creative_link_description", redacted);
                adsBuilder.Add("ad_creative_link_title", redacted);
                adsBuilder.Add("ad_creative_link_caption", redacted);
                adsBuilder.Add("ad_creative_body", redacted);
            } else {
                adsBuilder.Add("ad_creative_link_description", ad.ad->linkDescription);
                adsBuilder.Add("ad_creative_link_title", ad.ad->linkTitle);
                adsBuilder.Add("ad_creative_link_caption", ad.ad->linkCaption);
                adsBuilder.Add("ad_creative_body", ad.ad->body);
            }
            adsBuilder.Add("guestimateImpressions", ad.guestimateImpressions);
            adsBuilder.Add("guestimateSpendGBP", ad.guestimateSpend);
            adsBuilder.EndObject();
        }
        adsBuilder.EndArray();
        adsFile << adsBuilder.GetAndClear() << std::endl;
        adsFile.close();
    }
    summaryBuilder.EndArray();
    summaryFile << summaryBuilder.GetAndClear() << std::endl;
    summaryFile.close();
}

void DbUtils::WriteDbToDisk(AdDb &db, const std::string &fname) {
    std::fstream summaryFile(fname, std::ios_base::out);
    summaryFile << db.Serialize().json;
    summaryFile.close();
}

void DbUtils::WriteBreakdown(const Reports::BreakdownReport& report, const std::string &path) {
    std::fstream breakdownFile(path, std::ios_base::out);
    SimpleJSONPrettyBuilder breakdownBuilder;
    breakdownBuilder.Add("funders", report.keys);

    const auto AddObject = [&] (const std::string& name, std::function<void ()> inner) {
        breakdownBuilder.AddName(name);
        breakdownBuilder.StartAnonymousObject();
        inner();
        breakdownBuilder.EndObject();
    };

    const auto AddAnnonObject = [&] (std::function<void ()> inner) {
        breakdownBuilder.StartAnonymousObject();
        inner();
        breakdownBuilder.EndObject();
    };

    const auto AddArray = [&] (const std::string& name, std::function<void ()> inner) {
        breakdownBuilder.StartArray(name);
        inner();
        breakdownBuilder.EndArray();
    };

    using VennSet = std::vector<Reports::VennSet>;
    const auto DoBreakDown = [&] (const std::string& dataSet, const VennSet& views, const VennSet& spend) {
        AddObject(dataSet, [&] () {
            AddObject("impressions", [&] () {
                for (size_t keyIdx = 0; keyIdx < report.keys.size(); ++keyIdx) {
                    const std::string& key = report.keys[keyIdx];
                    const auto& issueViews = views[keyIdx];
                    AddObject(key, [&] () {
                        breakdownBuilder.Add("name", key);
                        breakdownBuilder.Add("type", std::string("venn"));
                        AddArray("data", [&] () {
                            for (const auto& issue: issueViews.data) {
                                AddAnnonObject([&] () {
                                    breakdownBuilder.Add("value", issue.value);
                                    std::vector<std::string> sets;
                                    sets.resize(issue.groups.size());
                                    for (size_t i = 0; i < sets.size(); ++i) {
                                        sets[i] = issueViews.groupNames[issue.groups[i]];
                                    }
                                    breakdownBuilder.Add("sets", sets);
                                });
                            }
                        });
                    });
                }
            });
            AddObject("spend", [&] () {
                for (size_t keyIdx = 0; keyIdx < report.keys.size(); ++keyIdx) {
                    const std::string& key = report.keys[keyIdx];
                    const auto& issueSpend = spend[keyIdx];
                    AddObject(key, [&] () {
                        breakdownBuilder.Add("name", key);
                        breakdownBuilder.Add("type", std::string("venn"));
                        AddArray("data", [&] () {
                            for (const auto& issue: issueSpend.data) {
                                AddAnnonObject([&] () {
                                    breakdownBuilder.Add("value", issue.value);
                                    std::vector<std::string> sets;
                                    sets.resize(issue.groups.size());
                                    for (size_t i = 0; i < sets.size(); ++i) {
                                        sets[i] = issueSpend.groupNames[issue.groups[i]];
                                    }
                                    breakdownBuilder.Add("sets", sets);
                                });
                            }
                        });
                    });
                }
            });
        });
    };
    const auto DoPie = [&] (const std::string& dataSet) {
        AddObject(dataSet, [&] () {
            AddObject("impressions", [&] () {
                for (size_t keyIdx = 0; keyIdx < report.keys.size(); ++keyIdx) {
                    const std::string& key = report.keys[keyIdx];
                    const Reports::PieMap& funderPages = report.pageViews[keyIdx];
                    AddObject(key, [&] () {
                        breakdownBuilder.Add("name", key);
                        breakdownBuilder.Add("type", std::string("pie"));
                        AddArray("data", [&] () {
                            for (const auto& pair: funderPages) {
                                AddAnnonObject([&] () {
                                    breakdownBuilder.Add("name", pair.first);
                                    breakdownBuilder.Add("y", pair.second);
                                });
                            }
                        });
                    });
                }
            });
            AddObject("spend", [&] () {
                for (size_t keyIdx = 0; keyIdx < report.keys.size(); ++keyIdx) {
                    const std::string& key = report.keys[keyIdx];
                    const Reports::PieMap& funderPages = report.pageSpend[keyIdx];
                    AddObject(key, [&] () {
                        breakdownBuilder.Add("name", key);
                        breakdownBuilder.Add("type", std::string("pie"));
                        AddArray("data", [&] () {
                            for (const auto& pair: funderPages) {
                                AddAnnonObject([&] () {
                                    breakdownBuilder.Add("name", pair.first);
                                    breakdownBuilder.Add("y", pair.second);
                                });
                            }
                        });
                    });
                }
            });
        });
    };

    DoBreakDown("Issues", report.issueViews, report.issueSpend);
    DoBreakDown("Cons", report.consViews, report.consSpend);
    DoPie("Pages");



    breakdownFile << breakdownBuilder.GetAndClear();
    breakdownFile.close();
}
