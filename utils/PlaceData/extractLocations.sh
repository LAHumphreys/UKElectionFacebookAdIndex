#!/bin/bash

#cat IPN_GB_2019.csv | grep ',"LOC",' | awk 'BEGIN {FS="\"" } {print $4 "----" $52 }' | head -n 5
cat IPN_GB_2019.csv | grep ',"LOC",' | awk -f groupIntoCon.awk
