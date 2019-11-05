BEGIN {
    FS="\"";
}

{
    loc=$4
    con=$52

    if ( con in cons ) {
          cons[con] = cons[con] ", \"" loc "\""
    } else {
          cons[con] = "{ \"id\": \"" con "\", \"keys\": [\"" loc "\""
    }
}

END {
    for (con in cons) {
        if (con != "" ) {
            print cons[con] "] },"
        }
    }
}
