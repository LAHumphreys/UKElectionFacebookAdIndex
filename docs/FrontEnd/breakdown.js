let data = {};

function GetDataUrl() {
    return "data/SpendBreakdown/Breakdown.json"
}

function UpdateGraph() {
    let formatter = null;
    let label = "";
    if (GetDataSubSet() == "spend") {
        formatter = function () {
            return "£" + this.value;
        };
        label = "£s";
    } else {
        formatter = function () {
            return this.value;
        };
        label = "Views";
    }

    let funder = GetFunder();
    let breakdown = GetDataSubSet();

    Highcharts.chart('ConsGraph', {
        title: {
            text: 'UK 2019 Election "' + funder + '" Advertising - Breakdown by ' + 'Consituency'
        },
        subtitle: {
            text: 'Source: Facebook Ads Archive'
        },
        series: data["Cons"][breakdown][funder]
    });

    Highcharts.chart('IssuesGraph', {
        title: {
            text: 'UK 2019 Election "' + funder + '" Advertising - Breakdown by ' + 'Issue'
        },
        subtitle: {
            text: 'Source: Facebook Ads Archive'
        },
        series: data["Issues"][breakdown][funder]
    });

    Highcharts.chart('PagesGraph', {
        title: {
            text: 'UK 2019 Election "' + funder + '" Advertising - Breakdown by ' + 'Advertised Page'
        },
        subtitle: {
            text: 'Source: Facebook Ads Archive'
        },
        series: data["Pages"][breakdown][funder]
    });
}

function UpdateData() {
    $.ajax({
        url: GetDataUrl()
    }).done(function (d) {
        data = d;
        SetValidFunders(d.funders);
    }).fail(function () {
        console.log("Failed to get: " + ref);
    });
}

