#include <FacebookParser.h>
#include <FacebookAd.h>
#include <iostream>

std::ostream &operator<<(std::ostream &os, const RegionCode &code) {
    switch (code) {

        case RegionCode::ENGLAND:
            os << "RegionCode::ENGLAND";
            break;
        case RegionCode::WALES:
            os << "RegionCode::WALES";
            break;
        case RegionCode::NORTHERN_IRELAND:
            os << "RegionCode::NORTHERN_IRELAND";
            break;
        case RegionCode::SCOTLAND:
            os << "RegionCode::SCOTLAND";
            break;
        case RegionCode::UNKNOWN:
            os << "RegionCode::UNKNOWN";
            break;
    }

    return os;
}

RegionCode ParseRegionCode(const std::string &code) {
    if (code == "England") {
        return RegionCode::ENGLAND;
    } else if (code == "Scotland") {
        return RegionCode::SCOTLAND;
    } else if (code == "Wales") {
        return RegionCode::WALES;
    } else if (code == "Northern Ireland") {
        return RegionCode::NORTHERN_IRELAND;
    } else {
        return RegionCode::UNKNOWN;
    }
}

std::ostream &operator<<(std::ostream &os, const GenderCode &code) {
    switch (code) {
        case GenderCode::FEMALE:
            os << "GenderCode::FEMALE";
            break;
        case GenderCode::MALE:
            os << "GenderCode::MALE";
            break;
        case GenderCode::UNKNOWN:
            os << "GenderCode::UNKNOWN";
            break;
    }

    return os;
}

GenderCode ParseGenderCode(const std::string &code) {
    if (code == "male") {
        return GenderCode::MALE;
    } else if (code == "female") {
        return GenderCode::FEMALE;
    } else {
        return GenderCode::UNKNOWN;
    }
}

std::ostream &operator<<(std::ostream &os, const AgeRangeCode &code) {
    switch (code) {
        case AgeRangeCode::AGE_18_TO_24:
            os << "AgeRangeCode:18-24";
            break;
        case AgeRangeCode::AGE_25_TO_34:
            os << "AgeRangeCode:25-34";
            break;
        case AgeRangeCode::AGE_35_TO_44:
            os << "AgeRangeCode:35-44";
            break;
        case AgeRangeCode::AGE_45_TO_54:
            os << "AgeRangeCode:45-54";
            break;
        case AgeRangeCode::AGE_55_TO_64:
            os << "AgeRangeCode:55-64";
            break;
        case AgeRangeCode::AGE_65_AND_OVER:
            os << "AgeRangeCode:65+";
            break;
        case AgeRangeCode::UNKNOWN:
            os << "AgeRangeCode:UNKNOWN";
            break;
    }

    return os;
}

AgeRangeCode ParseAgeRangeCode(const std::string &code) {
    if (code == "18-24") {
        return AgeRangeCode::AGE_18_TO_24;
    } else if (code == "25-34") {
        return AgeRangeCode::AGE_25_TO_34;
    } else if (code == "35-44") {
        return AgeRangeCode::AGE_35_TO_44;
    } else if (code == "45-54") {
        return AgeRangeCode::AGE_45_TO_54;
    } else if (code == "55-64") {
        return AgeRangeCode::AGE_55_TO_64;
    } else if (code == "65+") {
        return AgeRangeCode::AGE_65_AND_OVER;
    } else {
        return AgeRangeCode::UNKNOWN;
    }
}

bool Demographic::operator<(const Demographic &rhs) const {
    if (gender == rhs.gender) {
        return (age < rhs.age);
    } else {
        return (gender < rhs.gender);
    }
}
