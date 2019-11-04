//
// Created by lukeh on 03/11/2019.
//
#include <Reports.h>

using namespace Reports;

std::unique_ptr<Report> Reports::DoConsituencyReport(const AdDb &theDb) {
    auto report = std::make_unique<Report>();
    theDb.ForEachConsituency([&] (const std::string& conName) -> auto {
        auto ads = theDb.GetConstituency(conName);
        ReportItem& item = (*report)[conName];
        item.summary.name = conName;
        item.summary.count = ads.size();
        item.summary.estSpend = 0;
        item.summary.estImpressions = 0;

        item.ads.resize(ads.size());
        for (size_t i = 0; i < ads.size(); ++i) {
            auto& ad = *ads[i];
            auto& adReport = item.ads[i];
            if (ad.spend.lower_bound == 0 && ad.spend.upper_bound > 1) {
                adReport.guestimateSpend = 1;
                if (item.summary.estSpend == 0) {
                    item.summary.estSpend = 1;
                }
            } else {
                const size_t diff = (ad.spend.upper_bound - ad.spend.lower_bound);
                adReport.guestimateSpend = (ad.spend.lower_bound + (diff/2));
                item.summary.estSpend += adReport.guestimateSpend;
            }
            if (ad.impressions.lower_bound == 0 && ad.impressions.upper_bound > 1) {
                adReport.guestimateImpressions = 1;
            } else {
                const size_t diff = (ad.impressions.upper_bound - ad.impressions.lower_bound);
                adReport.guestimateImpressions = (ad.impressions.lower_bound + (diff/2));
            }
            item.summary.estImpressions += adReport.guestimateImpressions;
            adReport.ad = std::move(ads[i]);
        }

        return AdDb::DbScanOp::CONTINUE;
    });
    return report;
}
