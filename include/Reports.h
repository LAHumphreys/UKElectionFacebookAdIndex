//
// Created by lukeh on 03/11/2019.
//

#ifndef ELECTIONDATAANAL_REPORTS_H
#define ELECTIONDATAANAL_REPORTS_H
#include <string>
#include <vector>
#include <memory>
#include <AdDb.h>

namespace Reports {
    struct SummaryItem {
        std::string name;
        size_t count;
        size_t estSpend;
        size_t estImpressions;
    };
    struct AdReport {
        size_t guestimateSpend;
        size_t guestimateImpressions;
        std::shared_ptr<const FacebookAd> ad;
    };
    struct ReportItem {
        SummaryItem summary;
        std::vector<AdReport> ads;
    };
    using Report = std::map<std::string, ReportItem>;

    std::unique_ptr<Report> DoConsituencyReport(const AdDb& theDb);
}

#endif //ELECTIONDATAANAL_REPORTS_H
