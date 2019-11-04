#!/bin/bash


pushd ./cmake-build-release/
make -j 3  || exit 1
popd

rm -f FrontEnd/data/*/*.json
mkdir -p FrontEnd/data/Cons/
mkdir -p FrontEnd/data/Issues/
./cmake-build-release/processAds report.cfg data FrontEnd/data
