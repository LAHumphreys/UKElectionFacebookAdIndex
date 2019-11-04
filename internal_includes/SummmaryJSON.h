#ifndef ELECTIONDATAANAL_SUMMARYJSON_HE
#define ELECTIONDATAANAL_SUMMARYJSON_H

#include <SimpleJSON.h>

namespace SummaryJSON {
    NewUIntField(guestimateImpressions);
    NewUIntField(guestimateSpendGBP);
    NewStringField(name);
    NewUIntField(totalAds);

    typedef SimpleParsedJSON<
        guestimateImpressions,
        guestimateSpendGBP,
        name,
        totalAds
    > SummaryFields;
    NewObjectArray(summary, SummaryFields);

    typedef SimpleParsedJSON<
        summary
    > SummaryJSON;
}

#endif
