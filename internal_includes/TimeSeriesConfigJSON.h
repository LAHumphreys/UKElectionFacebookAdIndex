#ifndef __ELECTION_DATA_SERIES_CONFIG_H__
#define __ELECTION_DATA_SERIES_CONFIG_H__
#include <SimpleJSON.h>
namespace TimeSeriesCfg {
    NewStringField(file);
    NewStringField(label);
    typedef SimpleParsedJSON<file, label> Item;
    NewObjectArray(data, Item);

    typedef SimpleParsedJSON<data> Cfg;
}

#endif
