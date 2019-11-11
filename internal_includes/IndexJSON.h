//
// Created by lukeh on 11/11/2019.
//

#ifndef ELECTIONDATAANAL_INDEXJSON_H
#define ELECTIONDATAANAL_INDEXJSON_H
#include <SimpleJSON.h>

namespace IndexJSON {
    NewStringField (name)
    NewUI64ArrayField(keys);
    using Item = SimpleParsedJSON<name, keys>;
    NewObjectArray(items, Item);
    using JSON = SimpleParsedJSON<items>;
}

#endif //ELECTIONDATAANAL_INDEXJSON_H
