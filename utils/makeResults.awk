BEGIN {
    FS="	"
    currentCon = ""
        print "{"
}
{
    con=$1
    name = $2 ", " $3
    party = $5
    votes = $6

    if ( con != currentCon ) {
        if (currentCon != "") {
            print "        }]"
            print "    }],"
        }
        print "    \"" con "\": [{" 
        print "        \"name\": \"" con " 2017 Result\","
        print "        \"data\": [{"
        currentCon = con
    } else {
        print "        }, {"
    }
    print     "            \"name\": \"" party "\","
    print     "            \"y\": " votes

}

END {
        print "        }]"
        print "    }]"
        print "}"
}
