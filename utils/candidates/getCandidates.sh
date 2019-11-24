#!/bin/bash

baseUrl="https://whocanivotefor.co.uk"
indexUrl="$baseUrl/elections/parl.2019-12-12"

conUrls=$(wget $indexUrl -O - 2>/dev/null | egrep '<a href="/elections/parl' | awk 'BEGIN {FS="\""} {print $2}');

con=""
for conUrl in $conUrls; do
    if [[ "$con" != "" ]]; then
        echo ","
    fi
    href="$baseUrl$conUrl"
    con=$(wget $href  -O - 2>/dev/null | awk -f conParser.awk)
    #remove , UK from con name
    con=$(echo $con | sed -e 's/, UK"/"/')
    # Expand out html codes
    con=$(echo $con | sed -e "s/&#39;/'/g")
    echo -n $con
done
echo
