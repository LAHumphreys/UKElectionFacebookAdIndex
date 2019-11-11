//
// Created by lukeh on 12/11/2019.
//

#ifndef ELECTIONDATAANAL_DBJSON_H
#define ELECTIONDATAANAL_DBJSON_H
#include <SimpleJSON.h>

namespace DbJSON {
    NewStringField(store);
    NewStringField(cons);
    NewStringField(issues);
    using JSON = SimpleParsedJSON<store, cons, issues>;
}

#endif //ELECTIONDATAANAL_DBJSON_H
