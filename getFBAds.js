#!/usr/bin/nodejs
/*global $, console */

const request = require('request');
const util = require('util');
const fs = require('fs');

function Proxy() {
    this._jar = request.jar();
}

Proxy.prototype = {
    _jar: null,

    get: function(url, cb) {
        console.log(">> GET: " + url);
        request.get({
            url: url,
            jar: this._jar
        }, cb);
    },

    post: function (url, postObj, cb) {
        let me = this;
        request.post({
            url: url,
            json: true,
            body: postObj,
            jar: this._jar,
            followAllRedirects: true
        }, cb);
    }
}

// One will suffice...
let proxy = new Proxy();


let facebookurl="https://graph.facebook.com/v5.0/ads_archive"

facebookurl+="?access_token=" + process.env.FB_API_TOKEN
facebookurl+="&ad_reached_countries=['GB']"
facebookurl+="&search_terms=''"
facebookurl+="&ad_type=POLITICAL_AND_ISSUE_ADS"
facebookurl+="&ad_active_status=ALL"
facebookurl+="&impression_condition=HAS_IMPRESSIONS_LAST_90_DAYS"
facebookurl+="&fields=ad_snapshot_url,funding_entity,region_distribution,ad_delivery_start_time,ad_delivery_stop_time,ad_creative_link_description,ad_creative_link_title,ad_creative_link_caption,ad_creative_body,ad_creation_time,spend,impressions,demographic_distribution"
facebookurl+="&limit=250"

function Timestamp() {
    let date = new Date()
    let stamp = "";
    stamp += date.getFullYear();
    stamp += date.getMonth() +1;
    stamp += date.getDate();

    stamp += "T";
    if (date.getHours() < 10) {
        stamp += '0' + date.getHours();
    } else {
        stamp += date.getHours();
    }

    if (date.getMinutes() < 10) {
        stamp += '0' + date.getMinutes();
    } else {
        stamp += date.getMinutes();
    }

    if (date.getSeconds() < 10) {
        stamp += '0' + date.getSeconds();
    } else {
        stamp += date.getSeconds();
    }

    return stamp;
}

let basename="data/FacebookAds_"+ Timestamp();

function DumpFile(page, body) {
    let fname = basename + "_" + page
    fs.writeFile(fname, body, function (err, file) {
        if (err) {
            throw err
        }
    });
}

let count = 0;
let pages = 0;
function callback(payload, response, body) {
    pbody = JSON.parse(body);

    if ("data" in pbody) {
        pages+=1;
        count+=pbody['data'].length;

        DumpFile(pages, body);

        console.log("Page " + pages + ": " + count);
        if ("paging" in pbody) {
            let next = pbody['paging']['next'];
            proxy.get(next, callback);
        }
    } else {
        console.log ("ERROR: Failed to get data from the Facebook Ad API");
        console.log (body);
        process.exit(1)
    }

}
proxy.get(facebookurl, callback);
