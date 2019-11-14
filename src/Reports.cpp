//
// Created by lukeh on 03/11/2019.
//
#include <Reports.h>

using namespace Reports;
namespace {
    using KeyProvider = std::function<void (const AdDb::ForEachItemDefn& itemLoop)>;
    using ItemProvider = std::function<AdDb::FacebookAdList (const std::string& name)>;

    std::unique_ptr<Report> DoReport(
            const AdDb &theDb,
            const KeyProvider& source,
            const ItemProvider& getter)
    {
        auto report = std::make_unique<Report>();
        source([&] (const std::string& conName) -> auto {
            auto ads = getter(conName);
            ReportItem &item = (*report)[conName];
            item.summary.name = conName;
            item.summary.count = ads.size();
            item.summary.estSpend = 0;
            item.summary.estImpressions = 0;

            item.ads.resize(ads.size());
            for (size_t i = 0; i < ads.size(); ++i) {
                auto &ad = *ads[i];
                auto &adReport = item.ads[i];
                if (ad.spend.lower_bound == 0 && ad.spend.upper_bound > 1) {
                    adReport.guestimateSpend = 1;
                    if (item.summary.estSpend == 0) {
                        item.summary.estSpend = 1;
                    }
                } else {
                    const size_t diff = (ad.spend.upper_bound - ad.spend.lower_bound);
                    adReport.guestimateSpend = (ad.spend.lower_bound + (diff / 2));
                    item.summary.estSpend += adReport.guestimateSpend;
                }
                if (ad.impressions.lower_bound == 0 && ad.impressions.upper_bound > 1) {
                    adReport.guestimateImpressions = 1;
                } else {
                    const size_t diff = (ad.impressions.upper_bound - ad.impressions.lower_bound);
                    adReport.guestimateImpressions = (ad.impressions.lower_bound + (diff / 2));
                }
                item.summary.estImpressions += adReport.guestimateImpressions;
                adReport.ad = std::move(ads[i]);
            }

            return AdDb::DbScanOp::CONTINUE;
        });

        return report;
    }
}

std::unique_ptr<Report> Reports::DoConsituencyReport(const AdDb &theDb) {
    auto source = [&] (const auto& processor) -> void {
        theDb.ForEachConsituency(processor);
    };
    auto getter = [&] (const std::string& name) -> auto {
        return theDb.GetConstituency(name);
    };
    return DoReport(theDb, source, getter);
}

std::unique_ptr<Report> Reports::DoIssueReport(const AdDb &theDb) {
    auto source = [&] (const auto& processor) -> void {
        theDb.ForEachIssue(processor);
    };
    auto getter = [&] (const std::string& name) -> auto {
        return theDb.GetIssue(name);
    };
    return DoReport(theDb, source, getter);
}

std::unique_ptr<Reports::Report> Reports::DoDiffReport(const AdDb &start, const AdDb &end) {
    auto report = std::make_unique<Report>();
    start.ForEachConsituency([&] (const std::string& name) -> AdDb::DbScanOp {
        auto& con = (*report)[name];
        con.summary.name = name;
        auto startAds = start.GetConstituency(name);
        for (const auto& ad: startAds) {
            auto& adReport = con.ads.emplace_back();
            adReport.ad = ad;
            adReport.guestimateSpend = 0;
            adReport.guestimateImpressions = 0;
        }
        return AdDb::DbScanOp::CONTINUE;
    });
    return report;
}
