//
// Created by lukeh on 04/11/2019.
//

#ifndef ELECTIONDATAANAL_CONFIGPARSER_H
#define ELECTIONDATAANAL_CONFIGPARSER_H
#include <SimpleJSON.h>

namespace Config {
    NewStringField(id);
    NewStringArrayField(keys);

    using Item = SimpleParsedJSON<id, keys>;
    NewObjectArray(consituencies, Item);
    NewObjectArray(issues, Item);

    using DbConfig = SimpleParsedJSON<consituencies, issues>;
}

#endif //ELECTIONDATAANAL_CONFIGPARSER_H
