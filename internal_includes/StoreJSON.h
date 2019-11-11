//
// Created by lukeh on 12/11/2019.
//
#ifndef ELECTIONDATAANAL_STOREJSON_H
#define ELECTIONDATAANAL_STOREJSON_H
#include <SimpleJSON.h>

namespace Store {
    NewUI64Field(key);
    NewStringField(ad);
    using StoreItemJSON = SimpleParsedJSON<key, ad>;

    NewObjectArray(store,StoreItemJSON);
    using JSON = SimpleParsedJSON<store>;
}

#endif //ELECTIONDATAANAL_STOREJSON_H
