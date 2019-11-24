BEGIN {
    gotSet=0
    checking=0
    FS="\""
}

/"consituencies" *:/ {
    checking=1;
}

/"*\], *$/ {
        if (checking == 1) {
            gotSet=1
        }
        checking=0
}

/"id" *:/ {
    if (checking == 1) {
        if (gotSet == 0) {
           masterSet[$4] = "OK"
        } else {
            if ($4 in masterSet) {
            } else {
               print "Extra ID: " $4
            }
        }
    }

}
