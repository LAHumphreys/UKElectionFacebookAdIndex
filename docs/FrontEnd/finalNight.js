let _data = {};
let _votes = {};

function ClearTable(table) {
    if ($.fn.dataTable.isDataTable(table)) {
        var resultsTable = table.DataTable();
        resultsTable.clear();
        resultsTable.draw();
    }
}

function GetResult(conName) {
    let winner = {
        party: "",
        votes: 0,
        candidate: "???"
    };
    let runnerUp = {
        party: "",
        votes: 0,
        candidate: "???"
    };
    if (!(conName in _votes)) {
        console.log("WARNING: Could not find - " + conName);
    }
    _votes[conName][0].data.forEach(function (item, index) {
        if (item.y > winner.votes) {
            runnerUp.party = winner.party;
            runnerUp.votes = winner.votes;
            winner.party = item.name;
            winner.votes = item.y;
        } else if (item.y > runnerUp.votes) {
            runnerUp.party = item.name;
            runnerUp.votes = item.y;
        }

        if (runnerUp.votes === 0 && winner.votes !== 0 ) {
            runnerUp.votes = winner.votes;
            runnerUp.party = winner.party;
        }
    });
    winner.majority = (winner.votes - runnerUp.votes);

    return winner;

}
function LoadFilterTable(table, dataSet) {
    var data = [];
    _data.summary.forEach(function (item, index) {
        winner = GetResult(item.name);
        row = [];
        row.push(item.rank);
        row.push(item.name);
        row.push(winner.party);
        row.push(winner.candidate);
        row.push(Number(winner.majority).toLocaleString('en'));
        row.push(Number(item.guestimateImpressions).toLocaleString('en', {maximumSignificantDigits: 2}));
        let button = '<button onClick="LoadResults(\'' + item.name + '\')" type="button" class="btn btn-primary">View Data</button>';
        row.push(button);
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
            pageLength: 5,
            order: [[0, "asc"]]
        });
    }
}

function LoadResults(conName) {
    LoadResultsPie(conName);
    LoadViewPie(conName);
}


function LoadSummary() {
    $.ajax({
        url: GetSummaryUrl()
    }).done(function (data) {
        _data = data;
        let selectionTable = $('#selectionTable');
        LoadFilterTable(selectionTable);
        LoadResults(data.summary[0].name);
    }).fail(function () {
        console.log("Failed to load summary data!");
    });
}

function ResetData() {
    $.ajax({
        url: GetVoteUrl()
    }).done(function (data) {
        _votes = data;
        LoadSummary();
    }).fail(function () {
        console.log("Failed to load vote data!");
    });
}

function LoadResultsPie(conName) {
    Highcharts.chart('2017Pie', {
        chart: {
            plotBackgroundColor: null,
            plotBorderWidth: null,
            plotShadow: false,
            type: 'pie'
        },
        title: {
            text: 'UK 2017 Election Result: "' + conName + '"'
        },
        subtitle: {
            text: 'Source: TODO'
        },
        plotOptions: {
            pie: {
                allowPointSelect: true,
                cursor: 'pointer',
            }
        },
        series: _votes[conName]
    });
}

function LoadViewPie(conName) {
    Highcharts.chart('ViewsPie', {
        chart: {
            plotBackgroundColor: null,
            plotBorderWidth: null,
            plotShadow: false,
            type: 'pie'
        },
        title: {
            text: 'UK 2019 Facebook Ad Views (final 36 hours): "' + conName + '"'
        },
        subtitle: {
            text: 'Source: <a href="https://github.com/LAHumphreys/UKElectionFacebookAdIndex"> UK Election Facebook Ad Index </a>'
        },
        plotOptions: {
            pie: {
                allowPointSelect: true,
                cursor: 'pointer',
            }
        },
        series: _data.views[conName]
    });
}

