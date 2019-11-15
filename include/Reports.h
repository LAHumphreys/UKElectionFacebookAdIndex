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
    struct DbHasRegressed: public std::exception {
        const char* what() const noexcept override  {
            return "Ad impressions from the oldDb are missing in the new Db!";
        }
    };

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

    using FilterFunc = std::function<bool (const FacebookAd&)>;
    const FilterFunc AllowAll = [] (const FacebookAd&) -> bool { return true; };

    std::unique_ptr<Report> DoConsituencyReport(const AdDb& theDb, const FilterFunc& filter = AllowAll);

    std::unique_ptr<Report> DoIssueReport(const AdDb& theDb);
    std::unique_ptr<Report> DoDiffReport(const AdDb& start, const AdDb& end);
}

#endif //ELECTIONDATAANAL_REPORTS_H
