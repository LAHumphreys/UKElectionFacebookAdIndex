#ifndef ELECTIONDATAANAL_REPORT_AD_JSON_HE
#define ELECTIONDATAANAL_REPORT_AD_JSON_H

#include <SimpleJSON.h>
namespace ReportJSON {
        NewStringField(ad_creation_time);
        NewStringField(ad_creative_body);
        NewStringField(ad_creative_link_caption);
        NewStringField(ad_creative_link_description);
        NewStringField(ad_creative_link_title);
        NewStringField(ad_delivery_start_time);
        NewStringField(ad_delivery_end_time);
        NewStringField(funding_entity);
        NewUIntField(guestimateImpressions);
        NewUIntField(guestimateSpendGBP);

    namespace data_fields {
        typedef SimpleParsedJSON<
                ad_creation_time,
                ad_creative_body,
                ad_creative_link_caption,
                ad_creative_link_description,
                ad_creative_link_title,
                ad_delivery_start_time,
                ad_delivery_end_time,
                funding_entity,
                guestimateImpressions,
                guestimateSpendGBP
        > JSON;
    }
    NewObjectArray(data, data_fields::JSON);

    typedef SimpleParsedJSON<
            data
    > ReportJSON;
}

#endif
