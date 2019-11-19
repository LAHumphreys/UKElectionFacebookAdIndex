data = {}

function LoadCattegories(list) {

}

function GetDataUrl() {
    return "https://raw.githubusercontent.com/LAHumphreys/UKElectionFacebookAdIndex/master/docs/FrontEnd/data/TimeSeries/"  + GetDataSet() + ".json";
}

function UpdateGraph() {
    let catt = GetCattegory();
    Highcharts.chart('container', {
        chart: {
            type: 'area'
        },
        title: {
            text: 'UK 2019 Election "' + catt + '" Advertising - Breakdown by Funding Entity'
        },
        subtitle: {
            text: 'Source: Facebook Ads Archive. Baseline date: Tuesday 12th November 2019'
        },
        xAxis: {
            categories: data.timeSeries,
            tickmarkPlacement: 'on',
            title: {
                enabled: false
            }
        },
        yAxis: {
            title: {
                text: '£s'
            },
            labels: {
                formatter: function () {
                    return "£" + this.value;
                }
            }
        },
        tooltip: {
            split: false,
            valueSuffix: ''
        },
        plotOptions: {
            area: {
                stacking: 'normal',
                lineWidth: 1,
            }
        },
        series: data[catt]
    });
}

function UpdateData() {
    let catt = GetCattegory();
    $.ajax({
        url: GetDataUrl()
    }).done(function (d) {
        data = JSON.parse(d);
        SetValidCattegories(data.cattegories);
    }).fail(function () {
        console.log("Failed to get: " + ref);
    });
}

