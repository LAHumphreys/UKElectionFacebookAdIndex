BEGIN {
    FS="\""
    type=""
    id=""
    keys=""
}

$2=="@type" {
    type=$4
}

$2=="address" {
    if ( type == "Place" ) {
        id=$4
    }
    type=""
}

$2=="name" {
    if ( type == "Person" ) {
        if (keys != "" ) {
            keys = keys ", "
        }
        keys = keys "\"" $4 "\""
    }
    type=""
}

END {
    print "{ \"id\": \"" id "\", \"keys\": [" keys "]}"
}
