
# UK Election 2019 - Facebook Ad Indexer

Index political ads running on the Facebook platforms, and produce reports breaking spending down by:
- Issue (Brexit, NHS,...)
- Constituencies being targeted by Ads
- Who's paying for Ads?

The latest raw data dump can be [viewed here](https://lahumphreys.github.io/UKElectionFacebookAdIndex/FrontEnd/FacebookBreakdown.html).

The index is dumped as a JSON file for further analysis, but a simple web viewer is provided to quickly explore the data:

![Screenshot_20191117-094925__01](https://raw.githubusercontent.com/LAHumphreys/UKElectionFacebookAdIndex/master/docs/Screenshot_20191117-094925__01.jpg)
![Screenshot_20191117-094925__02](https://raw.githubusercontent.com/LAHumphreys/UKElectionFacebookAdIndex/master/docs/Screenshot_20191117-094925__02.jpg)


## How it Works
### Introduction - Facebook Ad Library
Facebook provides an "ad_archive" API: [Ad Library API](https://www.facebook.com/ads/library/api/?source=archive-landing-page&session_id=109a94f7-9b5d-4445-86c0-f116581bff66) (The ads_archive endpoint).  This allows us to extract:
- The text copy of every ad
- The Page Name (if any) it is advertising
- The entity that funded it
- Approximate ranges for Ad Views & Money Spent on the Ad

### Identifying Issue Ads
The text of every scanned for issue keywords. The current set of issue keywords is defined in the [report config file](https://github.com/LAHumphreys/UKElectionFacebookAdIndex/blob/master/report.cfg).

Unlike the Constituency Ads, the issue key word list is small and targeted - any use of the word Brexit automatically results in an Ad being classified as a Brexit Ad.

### Identifying Constituency Ads

In a perfect world, we'd use the location breakdown data from Facebook to figure out who's viewing an Ad. The problem is that data isn't granular enough (we only get % of views in England, Wales, Scotland etc). As a result we have to do a little guesswork. The [report config file](https://github.com/LAHumphreys/UKElectionFacebookAdIndex/blob/master/report.cfg) for the indexer  lists each constituency:
- The constituency name
- The previous MP
- The list of candidates standing in the seat
- Entries from the ONS list of 10,000 UK place names that fall within the Constituency.

The idea is to then scan ads looking for matches on these keywords - but we have to be careful to avoid large numbers of false positives skewing the data.

### Constituency Searches - Removing ambiguous keys
The issue with having a large number of place name / person name keys is that many constituencies can have a town with the same name (e.g Beeston). As a result duplicate keys are removed from the search (see [CfgPurify::RemoveDuplicates](https://github.com/LAHumphreys/UKElectionFacebookAdIndex/blob/master/src/ConfigPurify.cpp)).

The de-duping algorithm is that:
- For each key that is listed in multiple constituencies
- Remove it from every constituency, except:
- Do not remove a key from a Constituency if it forms part of the Constituency name

### Constituency Searches - Removing sources of false positives
Even after de-duping, we can be left with place names which are common in standard English prose (e.g The Square). These must be removed to avoid false positives overwhelming the data (see [CfgPurify::RemoveBannedKeys](https://github.com/LAHumphreys/UKElectionFacebookAdIndex/blob/master/src/ConfigPurify.cpp)).

The algorithm to remove false positives relies on a dictionary of 10,000s of the most common English words. Keys are then purged if:

- [They exactly match an entry in the dictionary
- OR they are of the form "The <dictionary word>"
- OR they are of the form "<dictionary word>, The"]
- AND the key does not form part of the Constituency name

## Building the Indexer
### dependencies
- Modern `c++17` compatible `g++`
- cmake
- libboost-all-dev

 ### Build steps
`./buildDeps.sh`

 ` ./buildFromDeps.sh`

## Running the Indexer
1. Get an API token from Facebook (see [Ad Library API](https://www.facebook.com/ads/library/api/?source=archive-landing-page&session_id=109a94f7-9b5d-4445-86c0-f116581bff66)).
2. Provide an API_TOKEN.sh that populates the FB_API_TOKEN env variable (see [refreshReport.sh](https://github.com/LAHumphreys/UKElectionFacebookAdIndex/blob/master/refreshReport.bash) ).
3. `./refreshReport --secureDb <restartFile>`

The first index will be slow (typically ~6 mins) as it has to scan everything for the firs time. By using the secureDb arg we can avoid most of this work next time.

Data is dumped to docs/Frontend 

## Faster iterative reports

When you re-run the report sometime later, the text content of most ads will be the same. To avoid expensive re-indexing, provide the Restart file from earlier:

`./refreshReport.sh --loadDb <restartFile>`
