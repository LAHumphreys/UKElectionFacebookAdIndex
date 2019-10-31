#include <FacebookParser.h>
#include <../internal_includes/FacebookAdJSON.h>
#include <logger.h>

using namespace FacebookAdJSON;
using namespace FacebookAdJSON::data_fields;
using namespace nstimestamp;
using Parser = FacebookAdParser;
namespace {
    struct ParserData: public FacebookAdParser::InternalDataHandle {
        QueryResultJSON parser;

        std::stringstream intermediate;

        size_t NumAds() {
            return parser.Get<FacebookAdJSON::data>().size();
        }

        template<class Field>
        auto& GetAdField(size_t adIndex) {
            return parser.Get<FacebookAdJSON::data>()[adIndex]->template Get<Field>();
        }

        template<class SubObject, class Field>
        auto& GetAdObjectField(size_t adIndex) {
            return parser.Get<FacebookAdJSON::data>()[adIndex]->template Get<SubObject>(). template Get<Field>();
        }

        template<class SubObject, class Field, class Target>
        void ConvertAdObjectField(size_t adIndex, Target& target) {
            // TODO: This will be super painful on the profile buts its not worth solving
            //       until we've solved item - by - item array parsing...
            intermediate.clear();
            intermediate.str(GetAdObjectField<SubObject, Field>(adIndex));
            intermediate >> target;
        }

        template<class SubObject>
        void ReadBoundFields(size_t adIndex, BoundedQuantity& qty) {
            ConvertAdObjectField<SubObject, data_fields::lower_bound>( adIndex, qty.lower_bound);
            ConvertAdObjectField<SubObject, data_fields::upper_bound>( adIndex, qty.upper_bound);
        }

        template<class Field, class DestinationType>
        void MoveField(size_t adIndex, DestinationType& target) {
            target = std::move(GetAdField<Field>(adIndex));
        }

        template<class Field>
        void MoveField(size_t adIndex, Time& target) {
            target = GetAdField<Field>(adIndex);
        }
    };

    ParserData& GetParser(FacebookAdParser::InternalDataHandle& hndl) {
        return dynamic_cast<ParserData&>(hndl);
    }

    void ParseDemographics(size_t adIndex, ParserData& parserData, FacebookAd& destination) {
        auto& demoList = parserData.GetAdField<demographic_distribution>(adIndex);
        for (auto& demo: demoList) {
            Demographic demoKey;
            demoKey.gender =  ParseGenderCode(demo->Get<gender>());
            demoKey.age =  ParseAgeRangeCode(demo->Get<data_fields::age>());
            std::string numericDist = demo->Get<data_fields::percentage>();

            parserData.intermediate.clear();
            parserData.intermediate.str(numericDist);
            parserData.intermediate >> destination.demographicDist[demoKey];
        }
    }

    void ParseRegions(size_t adIndex, ParserData& parserData, FacebookAd& destination) {
        auto& regionList = parserData.GetAdField<region_distribution>(adIndex);
        for (auto& reg: regionList) {
            RegionCode code = ParseRegionCode(reg->Get<data_fields::region>());
            std::string numericDist = reg->Get<data_fields::percentage>();

            parserData.intermediate.clear();
            parserData.intermediate.str(numericDist);
            parserData.intermediate >> destination.regionDist[code];
        }
    }

    void ParseAd(size_t adIndex, vector<FacebookAd> &into, ParserData &parserData) {
        auto& ad = into.emplace_back();
        parserData.MoveField<ad_creation_time>(adIndex, ad.creationTime);
        parserData.MoveField<ad_creative_body>(adIndex, ad.body);
        parserData.MoveField<ad_creative_link_caption>(adIndex, ad.linkCaption);
        parserData.MoveField<ad_creative_link_description>(adIndex, ad.linkDescription);
        parserData.MoveField<ad_creative_link_title>(adIndex, ad.linkTitle);
        parserData.MoveField<ad_delivery_start_time>(adIndex, ad.deliveryStartTime);
        parserData.MoveField<ad_delivery_stop_time>(adIndex, ad.deliveryEndTime);
        parserData.MoveField<currency>(adIndex, ad.currency);
        parserData.MoveField<funding_entity>(adIndex, ad.fundingEntity);
        parserData.MoveField<page_name>(adIndex, ad.pageName);

        parserData.ReadBoundFields<impressions>(adIndex, ad.impressions);
        parserData.ReadBoundFields<spend>(adIndex, ad.spend);

        ParseRegions(adIndex, parserData, ad);
        ParseDemographics(adIndex, parserData, ad);
    }

}
FacebookAdParser::FacebookAdParser() {
    internalData = std::make_unique<ParserData>();
}

Parser::ParseResult Parser::ParseFacebookAdQuery(const char *qryPage, std::vector<FacebookAd> &ads) {
    std::string error;
    ParseResult result = ParseResult::VALID;
    auto& parserData = GetParser(*this->internalData);

    parserData.parser.Clear();
    if (qryPage == nullptr) {
        result = ParseResult::INVALID_STRING;
    } else if (!parserData.parser.Parse(qryPage, error)) {
        result = ParseResult::PARSE_ERROR;
        SLOG_FROM(LOG_WARNING, __func__,
                "Failed to parse qry json: " << qryPage << std::endl
                 << "Error was: " << error);
    } else {
        for (size_t i = 0; i < parserData.NumAds(); ++i) {
            ParseAd(i, ads, parserData);
        }
    }

    return result;
}

