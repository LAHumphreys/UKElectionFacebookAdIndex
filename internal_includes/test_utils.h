#ifndef ELECTIONDATAANAL_TEST_UTILS_H
#define ELECTIONDATAANAL_TEST_UTILS_H
#include <FacebookParser.h>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const FacebookAdParser::ParseResult& result) {
    switch (result) {
        case FacebookAdParser::ParseResult::VALID:
            os << "ParseResult::VALID";
            break;
        case FacebookAdParser::ParseResult::INVALID_STRING:
            os << "ParseResult::INVALID_STRING";
            break;
        case FacebookAdParser::ParseResult::PARSE_ERROR:
            os << "ParseResult::PARSE_ERROR";
            break;
    }

    return os;
}

#endif //ELECTIONDATAANAL_TEST_UTILS_H
