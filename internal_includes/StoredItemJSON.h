//
// Created by lukeh on 11/11/2019.
//
#ifndef ELECTIONDATAANAL_STOREDITEMJSON_H
#define ELECTIONDATAANAL_STOREDITEMJSON_H
#include <SimpleJSON.h>

namespace StoredItem {
    NewStringField(item);
    NewStringField(upperTitle);
    NewStringField(upperBody);
    NewStringField(upperDescription);
    NewStringField(upperCaption);
    NewStringField(upperFundingEntity);
    NewStringField(upperPageName);

    using JSON = SimpleParsedJSON<
            item,
            upperTitle,
            upperBody,
            upperDescription,
            upperCaption,
            upperFundingEntity,
            upperPageName>;
};

#endif //ELECTIONDATAANAL_STOREDITEMJSON_H
