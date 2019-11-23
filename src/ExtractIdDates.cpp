
#include <iostream>
#include <FacebookParser.h>
#include <OSTools.h>
#include <fstream>

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: extractIds <dataDir>" << std::endl;
        return 1;
    }
    std::string dataDir = argv[1];

    FacebookAdParser parser;
    auto files = OS::Glob(dataDir + "/*");
    for (const std::string& path: files) {
        std::vector<std::unique_ptr<FacebookAd>> ads;
        std::ifstream file(path);
        if (parser.Parse(file, ads) != FacebookAdParser::ParseResult::VALID) {
            std::cout << "Failed to parse ad file: " << path << std::endl;
            return 1;
        }
        for (auto& ad: ads) {
            std::cout << ad->id << ": "
                      << ad->deliveryStartTime.Timestamp() << " -> " << ad->deliveryEndTime.Timestamp()
                      << std::endl;
        }
    }
    return 0;
}
