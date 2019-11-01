//
// Created by lukeh on 01/11/2019.
//

#ifndef ELECTIONDATAANAL_INDEXCONFIG_H
#define ELECTIONDATAANAL_INDEXCONFIG_H

#include <vector>
#include <string>

struct IndexConfig {
    struct Item {
        std::string id;
        std::vector<std::string> keys;
    };
    IndexConfig(std::vector<Item> items);
    IndexConfig& operator=(std::vector<Item> items);
    std::vector<Item> items;
};


#endif //ELECTIONDATAANAL_INDEXCONFIG_H
