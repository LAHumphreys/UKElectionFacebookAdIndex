//
// Created by lukeh on 24/11/2019.
//
#ifndef ELECTIONDATAANAL_TEST_BREAKDOWNPARSER_H
#define ELECTIONDATAANAL_TEST_BREAKDOWNPARSER_H

#include <SimpleJSON.h>

namespace TestBrkDwn {
    NewStringArrayField(funders);

    NewStringArrayField(sets);
    NewI64Field(value);
    NewStringField(name);
    NewStringField(type);

    using DataFields = SimpleParsedJSON<sets, value>;
    NewObjectArray(data, DataFields);

    using Funder = SimpleParsedJSON<name, type, data>;
    NewEmbededObject(TestFunder1, Funder);
    NewEmbededObject(TestFunder2, Funder);

    using ImpressionFields = SimpleParsedJSON<TestFunder1, TestFunder2>;
    using SpendFields = SimpleParsedJSON<TestFunder1, TestFunder2>;
    NewEmbededObject(impressions, ImpressionFields);
    NewEmbededObject(spend, SpendFields);

    using IssueFields = SimpleParsedJSON<impressions, spend>;
    NewEmbededObject(Issues, IssueFields);

    using BrkDwn = SimpleParsedJSON<funders, Issues>;
}

#endif //ELECTIONDATAANAL_TEST_BREAKDOWNPARSER_H
