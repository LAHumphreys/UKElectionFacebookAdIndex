//
// Created by lukeh on 03/11/2019.
//
#include <Reports.h>

using namespace Reports;
namespace {
    using KeyProvider = std::function<void (const AdDb::ForEachItemDefn& itemLoop)>;
    using ItemProvider = std::function<AdDb::FacebookAdList (const std::string& name)>;

    enum class MidPointMode {
        TRUE_MID,
        FLOOR_ZERO_BAND
    };
    constexpr size_t ZERO_BAND_FLOOR = 1;

    size_t MidPoint (const BoundedQuantity& qty, const MidPointMode& mode = MidPointMode::TRUE_MID) {
        if (mode == MidPointMode::FLOOR_ZERO_BAND && qty.lower_bound == 0) {
            return ZERO_BAND_FLOOR;
        } else {
            const size_t diff = (qty.upper_bound - qty.lower_bound);
            return (qty.lower_bound + (diff / 2));
        }
    }

    std::unique_ptr<Report> DoReport(
            const AdDb &theDb,
            const KeyProvider& source,
            const ItemProvider& getter,
            const Reports::FilterFunc& filter)
    {
        auto report = std::make_unique<Report>();
        source([&] (const std::string& conName) -> auto {
            auto ads = getter(conName);
            ReportItem &item = (*report)[conName];
            item.summary.name = conName;
            item.summary.count = 0;
            item.summary.estSpend = 0;
            item.summary.estImpressions = 0;

            item.ads.reserve(ads.size());
            for (size_t i = 0; i < ads.size(); ++i) {
                auto &ad = *ads[i];
                if (filter(ad)) {
                    item.summary.count++;
                    auto &adReport = item.ads.emplace_back();

                    adReport.guestimateSpend = MidPoint(ad.spend, MidPointMode::FLOOR_ZERO_BAND);
                    if (item.summary.estSpend == 0 || adReport.guestimateSpend != ZERO_BAND_FLOOR) {
                        item.summary.estSpend += adReport.guestimateSpend;
                    }

                    adReport.guestimateImpressions = MidPoint(ad.impressions, MidPointMode::FLOOR_ZERO_BAND);
                    item.summary.estImpressions += adReport.guestimateImpressions;

                    adReport.ad = std::move(ads[i]);
                }
            }

            return AdDb::DbScanOp::CONTINUE;
        });

        return report;
    }

    std::vector<Reports::AdReport> AdDiffs(
            const AdDb::FacebookAdList& startingAds,
            const AdDb::FacebookAdList& finalAds,
            Reports::SummaryItem& summary)
    {
        std::vector<Reports::AdReport> reports;
        auto startIt = startingAds.begin();
        auto finalIt = finalAds.begin();

        while (startIt != startingAds.end() || finalIt != finalAds.end()) {
            enum class Action {
                NEW_AD,
                UPDATED_AD
            };

            Action action;

            if (finalIt == finalAds.end()) {
                throw Reports::DbHasRegressed{};
            } else if (startIt == startingAds.end()) {
                    action = Action::NEW_AD;
            } else if ((*finalIt)->id < (*startIt)->id) {
                action = Action::NEW_AD;
            } else {
                action = Action::UPDATED_AD;
            }

            auto &adReport = reports.emplace_back();
            switch (action) {
                case Action::UPDATED_AD:
                    adReport.ad = *finalIt;
                    adReport.guestimateSpend = 0;
                    adReport.guestimateImpressions = 0;
                    ++startIt;
                    ++finalIt;
                    break;
                case Action::NEW_AD:
                    adReport.ad = *finalIt;
                    adReport.guestimateSpend = MidPoint((*finalIt)->spend, MidPointMode::FLOOR_ZERO_BAND);
                    adReport.guestimateImpressions = MidPoint((*finalIt)->impressions, MidPointMode::FLOOR_ZERO_BAND);
                    ++finalIt;
                    summary.count++;
                    break;
            }
            summary.estImpressions += adReport.guestimateImpressions;
            summary.estSpend += adReport.guestimateSpend;
        }
        return reports;
    }
}

std::unique_ptr<Report> Reports::DoConsituencyReport(
        const AdDb &theDb,
        const FilterFunc& filter)
{
    auto source = [&] (const auto& processor) -> void {
        theDb.ForEachConsituency(processor);
    };
    auto getter = [&] (const std::string& name) -> auto {
        return theDb.GetConstituency(name);
    };
    return DoReport(theDb, source, getter, filter);
}

std::unique_ptr<Report> Reports::DoIssueReport(const AdDb &theDb) {
    auto source = [&] (const auto& processor) -> void {
        theDb.ForEachIssue(processor);
    };
    auto getter = [&] (const std::string& name) -> auto {
        return theDb.GetIssue(name);
    };
    return DoReport(theDb, source, getter, Reports::AllowAll);
}

std::unique_ptr<Reports::Report> Reports::DoDiffReport(const AdDb &start, const AdDb &end) {
    auto report = std::make_unique<Report>();

    start.ForEachConsituency([&] (const std::string& name) -> AdDb::DbScanOp {
        auto& con = (*report)[name];
        con.summary.name = name;
        auto startAds = start.GetConstituency(name);
        auto finalAds = end.GetConstituency(name);
        con.ads = AdDiffs(startAds, finalAds, con.summary);

        return AdDb::DbScanOp::CONTINUE;
    });

    return report;
}
