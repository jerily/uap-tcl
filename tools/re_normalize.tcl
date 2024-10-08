# Copyright Jerily LTD. All Rights Reserved.
# SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
# SPDX-License-Identifier: MIT.

lappend auto_path [file join [file dirname [info script]] tcllib]

package require yaml

set data [::yaml::yaml2dict -file [file join [file dirname [info script]] .. uap-core regexes.yaml]]

set patch_regexp {

    {
        In this regexp '[ _\-;]? {0,2}' must be greedy, otherwise it will collapse to
        an empty match first. However, '.{0,200}' must be non-greedy. Thus, we can not make
        entire regexp non-greedy. Thus, we will try make '.{0,200}' more specific.
    }
    {\y(?:HTC/|HTC/[a-z0-9]{1,20}/|)HTC[ _\-;]? {0,2}(.{0,200}?)(?:-?Mozilla|fingerPrint|[;/\(\)]|$)}
    {\y(?:HTC/|HTC/[a-z0-9]{1,20}/|)HTC[ _\-;]? {0,2}([^;/\(\)=]{0,200}?)(?:-?Mozilla/|fingerPrint|[;/\(\)\{\}]|$)}

    {
        Here 'LG*' shoud be matched before 'LGE*'. But Tcl tries to provide the longest match first.
        Thus, we refactore the regexp, to match 'L', and then enable non-greedy mode.
    }
    {(?:^LG[\-]?|^LGE[\-/]?)([A-Za-z]+[0-9]+[A-Za-z]*)}
    {(?:^L{1,1}?(?:G[\-]?|GE[\-/]?))([A-Za-z]+[0-9]+[A-Za-z]*)(?:[;/\(\)_ ]|$)}

    {
        Here 'ZTE*' should be matched before 'ZTE-C*'. Thus we first match 'Z', and then
        enable non-greedy mode.
    }
    {; {0,2}(?:ZXY-ZTE_|ZTE\-U |ZTE[\- _]|ZTE-C[_ ])([^;/]{1,100}?)(?: Build|\) AppleWebKit)}
    {; {0,2}(?:Z{1,1}?(?:XY-ZTE_|TE\-U |TE[\- _](?!U)|TE[_ -](?=U[^ ])|TE_(?=U )|TE-C[_ ]))([^;/]{1,100}?)(?: Build|\) AppleWebKit)}

    {
        Tcl can not mix greedy and non-greedy quantifiers. Here we patch
        the 1st quantifier '{0,2}' to non-greedy version to make whole
        regexp non-greedy.
    }
    {(?i); {0,2}(ADR6200|ADR6400L|ADR6425LVW|Amaze|DesireS?|EndeavorU|Eris|EVO|Evo\d[A-Z]+|HD2|IncredibleS?|Inspire[A-Z0-9]*|Inspire[A-Z0-9]*|Sensation[A-Z0-9]*|Wildfire)[ _-](.{1,200}?)(?:[/;\)]|Build|MIUI|1\.0)}
    {(?i); {0,2}?(ADR6200|ADR6400L|ADR6425LVW|Amaze|DesireS?|EndeavorU|Eris|EVO|Evo\d[A-Z]+|HD2|IncredibleS?|Inspire[A-Z0-9]*|Inspire[A-Z0-9]*|Sensation[A-Z0-9]*|Wildfire)[ _-](.{1,200}?)(?:[/;\)]|Build|MIUI|1\.0)}

    {
        Tcl can not mix greedy and non-greedy quantifiers. Here we patch
        the 1st atom '(?:; {0,2}|^)' to non-greedy version to make whole
        regexp non-greedy.
    }
    {(?:; {0,2}|^)((?:GT\-[BIiNPS]\d{4}|I9\d{2}0[A-Za-z\+]?\y)[^;/\)]*?)(?:Build|Linux|MIUI|[;/\)])}
    {(?:; {0,2}|^){1,1}?((?:GT\-[BIiNPS]\d{4}|I9\d{2}0[A-Za-z\+]?\y)[^;/\)]*?)(?:Build|Linux|MIUI|[;/\)])}

    {
        Tcl can not mix greedy and non-greedy quantifiers. Here we patch
        the 1st atom 'A' to non-greedy version to make whole
        regexp non-greedy.
    }
    {Android[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,2}[A-Za-z]{2}[_\-][A-Za-z]{0,2}\-? {0,2}; {0,2}(.{1,200}?)( Build[/ ]|\))}
    {A{1,1}?ndroid[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,2}[A-Za-z]{2}[_\-][A-Za-z]{0,2}\-? {0,2}; {0,2}(.{1,200}?)( Build[/ ]|\))}

    {
        This and all other patches above are the same. They make entire regexp non-greedy.
    }
    {; {0,2}(ALLVIEW[ _]?|Allview[ _]?)((?:Speed|SPEED).{0,200}?)(?: Build|\) AppleWebKit)}
    {; {0,2}?(ALLVIEW[ _]?|Allview[ _]?)((?:Speed|SPEED).{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {Android[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,2}[A-Za-z]{0,2}\- {0,2}; {0,2}(.{1,200}?)( Build[/ ]|\))}
    {A{1,1}?ndroid[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,2}[A-Za-z]{0,2}\- {0,2}; {0,2}(.{1,200}?)( Build[/ ]|\))}

    {}
    {; {0,2}(?:ARCHOS|Archos) ?(GAMEPAD.{0,200}?)(?: Build|\) AppleWebKit)}
    {; {0,2}?(?:ARCHOS|Archos) ?(GAMEPAD.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {(?:ARCHOS|Archos)[ _]?(.{0,200}?)(?: Build|[;/\(\)\-]|$)}
    {(?:ARCHOS|Archos){1,1}?[ _]?(.{0,200}?)(?: Build|[;/\(\)\-]|$)}

    {}
    {; {0,2}(?:Cat)(Nova.{0,200}?)(?: Build|\) AppleWebKit)}
    {; {0,2}?(?:Cat)(Nova.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {(?i); {0,2}(CUBE[ _])?([KU][0-9]+ ?GT.{0,200}?|A5300)(?: Build|\) AppleWebKit)}
    {(?i);{1,1}? {0,2}(CUBE[ _])?([KU][0-9]+ ?GT.{0,200}?|A5300)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(?:Dell|DELL)[ _](Aero|Venue|Thunder|Mini.{0,200}?|Streak[ _]Pro)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(?:Dell|DELL)[ _](Aero|Venue|Thunder|Mini.{0,200}?|Streak[ _]Pro)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}Explay[_ ](.{1,200}?)(?:[\)]| Build)}
    {;{1,1}? {0,2}Explay[_ ](.{1,200}?)(?:[\)]| Build)}

    {}
    {; {0,2}(IQ.{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(IQ.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(Fly|FLY)[ _](IQ[^;]{1,100}?|F[34]\d+[^;]{0,100}?);?(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(Fly|FLY)[ _](IQ[^;]{1,100}?|F[34]\d+[^;]{0,100}?);?(?: Build|\) AppleWebKit)}

    {}
    {Android[\- ][\d]+\.[\d]+\-update1; [A-Za-z]{2}\-[A-Za-z]{0,2} {0,2}; {0,2}(.{1,200}?)( Build[/ ]|\))}
    {A{1,1}?ndroid[\- ][\d]+\.[\d]+\-update1; [A-Za-z]{2}\-[A-Za-z]{0,2} {0,2}; {0,2}(.{1,200}?)( Build[/ ]|\))}

    {}
    {(?i); {0,2}(Gionee)[ _\-]([^;/]{1,100}?)(?:/[^;/]{1,100}|)(?: Build|\) AppleWebKit)}
    {(?i);{1,1}? {0,2}(Gionee)[ _\-]([^;/]{1,100}?)(?:/[^;/]{1,100}|)(?: Build|\) AppleWebKit)}

    {}
    {(?i)(?:[;,] {0,2}|^)(?:htccn_chs-|)HTC[ _-]?([^;]{1,200}?)(?: {0,2}Build|clay|Android|-?Mozilla| Opera| Profile| UNTRUSTED|[;/\(\)]|$)}
    {(?i)(?:[;,] {0,2}|^){1,1}?(?:htccn_chs-|)HTC[ _-]?([^;]{1,200}?)(?: {0,2}Build|clay|Android|-?Mozilla| Opera| Profile| UNTRUSTED|[;/\(\)]|$)}

    {}
    {Android[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,2}[A-Za-z]{0,2}\- {0,2}; {0,2}(.{1,200}?)( Build[/ ]|\))}
    {A{1,1}?ndroid[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,2}[A-Za-z]{0,2}\- {0,2}; {0,2}(.{1,200}?)( Build[/ ]|\))}

    {}
    {; {0,2}(?:LNV-|)(?:=?[Ll]enovo[ _\-]?|LENOVO[ _])(.{1,200}?)(?:Build|[;/\)])}
    {;{1,1}? {0,2}(?:LNV-|)(?:=?[Ll]enovo[ _\-]?|LENOVO[ _])(.{1,200}?)(?:Build|[;/\)])}

    {}
    {; {0,2}(LG-)([A-Z]{1,2}\d{2,}[^,;/\)\(]*?)(?:Build| V\d+|[,;/\)\(]|$)}
    {;{1,1}? {0,2}(LG-)([A-Z]{1,2}\d{2,}[^,;/\)\(]*?)(?:Build| V\d+|[,;/\)\(]|$)}

    {}
    {; {0,2}(?:Polaroid[ _]|)((?:MIDC\d{3,}|PMID\d{2,}|PTAB\d{3,})[^;/]{0,30}?)(\/[^;/]{0,30}|)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(?:Polaroid[ _]|)((?:MIDC\d{3,}|PMID\d{2,}|PTAB\d{3,})[^;/]{0,30}?)(\/[^;/]{0,30}|)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(Q[Mm]obile)_([^_]+)_([^_]+?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(Q[Mm]obile)_([^_]+)_([^_]+?)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}([g|G]oogle)? (Pixel.{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}([g|G]oogle)? (Pixel.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(ARIZONA|(?:ATLAS|Atlas) W|D930|Grand (?:[SX][^;]{0,200}?|Era|Memo[^;]{0,200}?)|JOE|(?:Kis|KIS)\y[^;]{0,200}?|Libra|Light [^;]{0,200}?|N8[056][01]|N850L|N8000|N9[15]\d{2}|N9810|NX501|Optik|(?:Vip )Racer[^;]{0,200}?|RacerII|RACERII|San Francisco[^;]{0,200}?|V9[AC]|V55|V881|Z[679][0-9]{2}[A-z]?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(ARIZONA|(?:ATLAS|Atlas) W|D930|Grand (?:[SX][^;]{0,200}?|Era|Memo[^;]{0,200}?)|JOE|(?:Kis|KIS)\y[^;]{0,200}?|Libra|Light [^;]{0,200}?|N8[056][01]|N850L|N8000|N9[15]\d{2}|N9810|NX501|Optik|(?:Vip )Racer[^;]{0,200}?|RacerII|RACERII|San Francisco[^;]{0,200}?|V9[AC]|V55|V881|Z[679][0-9]{2}[A-z]?)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(Sprint )(.{1,200}?) {0,2}(?:Build|[;/])}
    {;{1,1}? {0,2}(Sprint )(.{1,200}?) {0,2}(?:Build|[;/])}

    {}
    {; {0,2}(BlackBird)[ _](.{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(BlackBird)[ _](.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {Android[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,3}\-?[A-Za-z]{2}; {0,2}(.{1,50}?)( Build[/ ]|\))}
    {A{1,1}?ndroid[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,3}\-?[A-Za-z]{2}; {0,2}(.{1,50}?)( Build[/ ]|\))}

    {}
    {; {0,2}(ImPAD) ?(\d+(?:.){0,100}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(ImPAD) ?(\d+(?:.){0,100}?)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(GenxDroid7|MSD7.{0,200}?|AX\d.{0,200}?|Tab 701|Tab 722)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(GenxDroid7|MSD7.{0,200}?|AX\d.{0,200}?|Tab 701|Tab 722)(?: Build|\) AppleWebKit)}

    {}
    {(?i); {0,2}(Cynus)[ _](F5|T\d|.{1,200}?) {0,2}(?:Build|[;/\)])}
    {(?i);{1,1}? {0,2}(Cynus)[ _](F5|T\d|.{1,200}?) {0,2}(?:Build|[;/\)])}

    {}
    {; ?(NEC-)(.{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? ?(NEC-)(.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(POMP)[ _\-](.{1,200}?) {0,2}(?:Build|[;/\)])}
    {;{1,1}? {0,2}(POMP)[ _\-](.{1,200}?) {0,2}(?:Build|[;/\)])}

    {}
    {; {0,2}(PLT[0-9]{4}.{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(PLT[0-9]{4}.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(SP[T]?\-\d{2}[^;/]*)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(SP[T]?\-\d{2}[^;/]*)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(ST\d{4}.{0,200})Build/ST}
    {;{1,1}? {0,2}(ST\d{4}.{0,200})Build/ST}

    {}
    {; {0,2}(ST\d{4}.{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(ST\d{4}.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {; (BASE) (lutea|Lutea 2|Tab[^;]{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? (BASE) (lutea|Lutea 2|Tab[^;]{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {Android[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,3}\-?[A-Za-z]{2}; {0,2}(.{1,50}?)( Build[/ ]|\))}
    {A{1,1}?ndroid[\- ][\d]+(?:\.[\d]+)(?:\.[\d]+|); {0,3}\-?[A-Za-z]{2}; {0,2}(.{1,50}?)( Build[/ ]|\))}

    {}
    {; {0,2}(?:meizu_|MEIZU )(.{1,200}?) {0,2}(?:Build|[;\)])}
    {;{1,1}? {0,2}(?:meizu_|MEIZU )(.{1,200}?) {0,2}(?:Build|[;\)])}

    {}
    {; {0,2}(SK\-.{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(SK\-.{0,200}?)(?: Build|\) AppleWebKit)}

    {}
    {(?i)(TOUCH(?:TAB|PAD).{1,200}?)(?: Build|\) AppleWebKit)}
    {(?i)(T{1,1}?OUCH(?:TAB|PAD).{1,200}?)(?: Build|\) AppleWebKit)}

    {}
    {; {0,2}(GTablet.{0,200}?)(?: Build|\) AppleWebKit)}
    {;{1,1}? {0,2}(GTablet.{0,200}?)(?: Build|\) AppleWebKit)}

}

proc quote { str } {
    set str [string map [list "\"" "\\\"" "\\" "\\\\"] $str]
    return "\"$str\""
}

proc fix_regexp { re ci } {

    # set before $re

    # check the statement like:
    #     token{m,n}
    # Tcl doesn't support m or n greater then 255. Replace m or n with 255
    # if we found a lager value in regexp.

    # 1. Try to find these statements
    set found [regexp -all -inline -indices {\{(\d+),(\d+)\}} $re]
    if { [llength $found] } {
        # 2. Now we have the result as a list. Each match is 3 list elements.
        #    Let's group the result by each match.
        set found [lmap { a1 a2 a3 } $found { list $a1 $a2 $a3 }]
        # 3. Now we need to reverse the list. If we start replacing the string
        # in regexp from the beginning, further match indices will be invalid.
        set found [lreverse $found]
        # 4. Go through matches
        foreach rec $found {
            lassign $rec idx_all idx_m idx_n
            set m [string range $re {*}$idx_m]
            set n [string range $re {*}$idx_n]
            if { $m > 255 || $n > 255 } {
                if { $m > 255 } { set m 255 }
                if { $n > 255 } { set n 255 }
                set re [string replace $re {*}$idx_all "{$m,$n}"]
            }
        }
    }

    # in js regexp \b is a word boundary and \B is not a word boundary
    # in Tcl corresponding constraints are \y and \Y
    set re [string map [list "\\b" "\\y"] $re]

    # Some regexps start with "^.{0,200}?"
    # This seems to be a hack to limit the search to the first 200 characters
    # and avoid high CPU utilization when dealing with very long strings.
    # However, this doesn't work very well in Tcl's regexp because it disables
    # greed mode for whole regexp and there is no way to turn it back on.
    # As a workaround, we will strip this from regexp, but we limit string
    # by 300 characters in uap_ParseCmd() function in library.c.
    set re [string map [list {^.{0,200}?} {}] $re]

    if { $ci } {
        set re "(?i)$re"
    }

    foreach { description _re _patch } $::patch_regexp {
        if { $re eq $_re } {
            set re $_patch
            break
        }
    }

    # if { $before ne $re } {
    #     puts stderr "Before RE: $before"
    #     puts stderr "After  RE: $re"
    # }

    return $re

}

set known_keys [dict create {*}{
    user_agent_parsers {
        family_replacement
        v1_replacement v2_replacement v3_replacement v4_replacement
    }
    os_parsers {
        os_replacement
        os_v1_replacement os_v2_replacement os_v3_replacement os_v4_replacement
    }
    device_parsers {
        device_replacement brand_replacement model_replacement
    }
}]

foreach rec [dict get $data [lindex $argv 0]] {
    incr count
    set opts [list]
    set ci [expr { [dict exists $rec "regex_flag"] && [string first "i" [dict get $rec "regex_flag"]] != -1 }]
    lappend opts [quote [fix_regexp [dict get $rec "regex"] $ci]]
    foreach key [dict get $known_keys [lindex $argv 0]] {
        if { [dict exists $rec $key] } {
            set val [dict get $rec $key]
            set placeholders [regexp -inline -all {\$(\d)} $val]
            if { ![llength $placeholders] } {
                set placeholders "NULL"
            } else {
                regsub -all {\$(\d)} $val {%s} val
                set placeholders [lmap { _ x } $placeholders { set x $x }]
                set placeholders [quote [join $placeholders {}]]
            }
            lappend opts [quote $val] $placeholders
        } else {
            lappend opts "NULL" "NULL"
        }
    }
    set opts [join $opts {, }]
    puts "UAPTCL_REGEX($opts)"
}

puts "#define UAPTCL_REGEX_COUNT $count"
