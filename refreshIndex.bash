#!/bin/bash


pushd ./cmake-build-release/
make -j 3  || exit 1
make test || exit 1
popd

echo -n "Cleaning Cfg..."
rm -f clean.cfg
./cmake-build-release/cfgClean report.cfg utils/words.txt clean.cfg > purged.txt || exit 1
echo "done, please review: purged.txt"

echo "Regenerating index..."
rm -f FrontEnd/data/*/*.json
mkdir -p FrontEnd/data/Cons/
mkdir -p FrontEnd/data/Issues/
DEV_TOOLS_LOG_OVERVIEW=YES time ./cmake-build-release/processAds clean.cfg data FrontEnd/data
