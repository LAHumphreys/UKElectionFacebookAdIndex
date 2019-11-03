#ifndef ELECTIONDATAANAL_INCLUDE_FACEBOOK_PARSER_H
#define ELECTIONDATAANAL_INCLUDE_FACEBOOK_PARSER_H

#include <vector>
#include <FacebookAd.h>
#include <memory>

class FacebookAdParser {
public:
    FacebookAdParser();

    enum class ParseResult {
        VALID,
        INVALID_STRING,
        PARSE_ERROR
    };

    ParseResult Parse(std::istream& source, std::vector<FacebookAd>& ads);
    ParseResult ParseFacebookAdQuery(const char* qryPage, std::vector<FacebookAd>& ads);

    struct InternalDataHandle {
        virtual ~InternalDataHandle() = default;
    };
private:
    // This is a nasty hack to prevent the compilation intensive JSON
    // parsing headers infecting the rest of the project
    std::unique_ptr<InternalDataHandle> internalData;
};

#endif
