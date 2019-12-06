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
    NewI64Field(y);
    NewStringField(name);
    NewStringField(type);


    namespace Venn {
        using DataFields = SimpleParsedJSON<sets, value>;
        NewObjectArray(data, DataFields);

        using Funder = SimpleParsedJSON<name, type, data>;
        NewEmbededObject(TestFunder1, Funder);
        NewEmbededObject(TestFunder2, Funder);

        using Data = SimpleParsedJSON<TestFunder1, TestFunder2>;
        NewEmbededObject(impressions, Data);
        NewEmbededObject(spend, Data);

        using Set = SimpleParsedJSON<impressions, spend>;
    }

    namespace Pie {
        using DataFields = SimpleParsedJSON<name, y>;
        NewObjectArray(data, DataFields);

        using Funder = SimpleParsedJSON<name, type, data>;
        NewEmbededObject(TestFunder1, Funder);
        NewEmbededObject(TestFunder2, Funder);

        using Data = SimpleParsedJSON<TestFunder1, TestFunder2>;
        NewEmbededObject(impressions, Data);
        NewEmbededObject(spend, Data);

        using Set = SimpleParsedJSON<impressions, spend>;
    }
    NewEmbededObject(Issues, Venn::Set);
    NewEmbededObject(Cons, Venn::Set);
    NewEmbededObject(Pages, Pie::Set);

    using BrkDwn = SimpleParsedJSON<funders, Issues, Cons, Pages>;
}

#endif //ELECTIONDATAANAL_TEST_BREAKDOWNPARSER_H
