function GetDataUrl(indexKey) {
    return "data/" + GetDataSet() + "/" + indexKey + ".json";
}
function ClearTable(table) {
    if ($.fn.dataTable.isDataTable(table)) {
        var resultsTable = table.DataTable();
        resultsTable.clear();
        resultsTable.draw();
    }
}
function LoadFilterTable(table, summary) {
    var data = [];
    summary.forEach(function (item, index) {
        let checkbox = '<div class="form-check">'
        checkbox += '<input class="form-check-input" type="checkbox" id="' + item.name + '" value="">';
        checkbox += '</div>';
        row = [checkbox];
        row.push(item.name);
        row.push(item.totalAds);
        row.push(item.guestimateImpressions);
        row.push(item.guestimateSpendGBP);
        data.push(row)
    });
    if ($.fn.dataTable.isDataTable(table)) {
        var resultsTable = table.DataTable();
        resultsTable.clear();
        resultsTable.rows.add(data);
        resultsTable.draw();
    } else {
        table.DataTable({
            data: data,
            pageLength: 5
        });
    }
}

function ConsolidateData(table, groupField, keys, cb) {
    let result = {};

    keys.forEach(function (key, index) {
        let ref = GetDataUrl(key);
        $.ajax({
            url: ref
        }).done(function (d) {
            d.data.forEach(function (ad, index) {
                let groupKey = ad[groupField];
                if (!(groupKey in result)) {
                    result[groupKey] = {
                        totalAds: 0,
                        guestimateImpressions: 0,
                        guestimateSpendGBP: 0,
                        ads: []
                    };
                }

                let group = result[groupKey];

                group.totalAds += 1;
                group.guestimateImpressions += ad.guestimateImpressions;
                group.guestimateSpendGBP += ad.guestimateSpendGBP;
                group.ads.push(ad);
            });
            cb(table, result);
        }).fail(function () {
            console.log("Failed to get: " + ref);
        });
    });

}

function GetAdList(ads) {
    let tableTemplate = `
    <table>
        <thead>
        <tr>
           <th> Link Title </th>
           <th> Link Caption </th>
           <th> Link Description </th>
           <th> Body </th>
           <th> Impressions  </th>
           <th> Spend  </th>
           </tr>
        </thead>
    </table>
    `;
    let table = $(tableTemplate);
    let data = [];
    ads.forEach(function (ad, idx) {
        let row = [];
        row.push(ad.ad_creative_link_title);
        row.push(ad.ad_creative_link_caption);
        row.push(ad.ad_creative_link_description);
        row.push(ad.ad_creative_body);
        row.push(ad.guestimateImpressions);
        row.push(ad.guestimateSpendGBP);
        data.push(row);
    });

    table.DataTable({
        data: data
    });

    return table;
}

function ConfigureViewAds(consolidation) {
    var resultsTable = $("#resultsTable").DataTable();

    // Replace the event handler (We don't want to fire twice!)
    let buttons = $("#resultsTable").find("button");
    buttons.off("click").click(function () {
        var tr = $(this).closest('tr');
        var row = resultsTable.row( tr );

        if ( row.child.isShown() ) {
            // This row is already open - close it
            row.child.hide();
        }
        else {
            // Open this row
            let groupKey = tr.find("td").first().next().text();
            row.child(GetAdList(consolidation[groupKey].ads));
            row.child.show();
        }
    } );
}


function DrawResults(table, consolidation) {
    var data = [];
    for (let groupKey in consolidation) {
        let row = ['<button type="button" class="btn btn-secondary btn-sm">View Ads</button>'];
        row.push(groupKey);
        let group = consolidation[groupKey];
        row.push(group.totalAds);
        row.push(group.guestimateImpressions);
        row.push(group.guestimateSpendGBP);
        data.push(row);
    }

    if ($.fn.dataTable.isDataTable(table)) {
        var resultsTable = table.DataTable();
        resultsTable.clear();
        resultsTable.rows.add(data);
        resultsTable.draw();
    } else {
        var resultsTable = table.DataTable({
            data: data,
        });
    }
    ConfigureViewAds(consolidation);
    resultsTable.on('draw', function () {
        ConfigureViewAds(consolidation);
    });
}

function LoadResults(table, dataList) {
    let consolidation = ConsolidateData(table, 'funding_entity', dataList, DrawResults);

}
