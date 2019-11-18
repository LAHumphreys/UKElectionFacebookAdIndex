#!/bin/bash

chmod -w dbs/*


pushd ./Build/
make -j 3  || exit 1
make test || exit 1
popd

echo -n "Cleaning Cfg..."
rm -f clean.cfg
./Build/cfgClean report.cfg utils/words.txt clean.cfg > purged.txt || exit 1
echo "done, please review: purged.txt"

echo "Regenerating index..."
rm -f docs/FrontEnd/data/*/*.json
mkdir -p docs/FrontEnd/data/Cons/
mkdir -p docs/FrontEnd/data/Issues/
DEV_TOOLS_LOG_OVERVIEW=YES ./Build/processAds $@ clean.cfg data docs/FrontEnd/data
