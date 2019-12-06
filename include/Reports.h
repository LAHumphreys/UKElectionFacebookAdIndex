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

    /**
     * An individual category (ReportItem) - can be broken down over time
     */
    struct TimeSeriesItem {
        std::map<std::string, std::vector<size_t>> guestimatedSpend;
        std::map<std::string, std::vector<size_t>> guestimatedImpressions;
        std::vector<size_t> residualSpend;
        std::vector<size_t> residualImpressions;
    };
    using TimeSeriesReport = std::map<std::string, TimeSeriesItem>;

    using FilterFunc = std::function<bool (const FacebookAd&)>;
    const FilterFunc AllowAll = [] (const FacebookAd&) -> bool { return true; };

    std::unique_ptr<Report> DoConsituencyReport(const AdDb& theDb, const FilterFunc& filter = AllowAll);

    std::unique_ptr<Report> DoIssueReport(const AdDb& theDb, const FilterFunc& filter = AllowAll);
    std::unique_ptr<Report> DoDiffReport(const AdDb& start, const AdDb& end);

    enum class TimeSeriesMode {
        STANDARD,
        REMOVE_BASELINE
    };
    std::unique_ptr<TimeSeriesReport> DoTimeSeries(const std::vector<Report*>& reports, const TimeSeriesMode& mode = TimeSeriesMode::STANDARD);

    struct VennSet {
        std::vector<std::string> groupNames;
        struct VennItem {
            std::vector<size_t> groups;
            size_t value;
        };
        std::vector<VennItem> data;
    };
    using PieMap = std::map<std::string, size_t>;
    struct BreakdownReport {
        std::vector<std::string> keys;
        std::vector<VennSet>     issueViews;
        std::vector<VennSet>     issueSpend;
        std::vector<VennSet>     consViews;
        std::vector<VennSet>     consSpend;
        std::vector<PieMap>      pageViews;
        std::vector<PieMap>      pageSpend;
    };
}

#endif //ELECTIONDATAANAL_REPORTS_H
