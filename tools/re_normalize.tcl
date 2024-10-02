# Copyright Jerily LTD. All Rights Reserved.
# SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
# SPDX-License-Identifier: MIT.

lappend auto_path [file join [file dirname [info script]] tcllib]

package require yaml

set data [::yaml::yaml2dict -file [file join [file dirname [info script]] .. uap-core regexes.yaml]]

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
    # in Tcl corresponding constraints are \m and \Y
    set re [string map [list "\\b" "\\m"] $re]

    # If regexp starts with "^.{0,200}?" then whole regexp will be non-greedy.
    # Let's try to avoid this by replacing it with "^(?:.{0,200})?"
    if { [string first {^.{0,200}?} $re] == 0 } {
        set re [string replace $re 0 9 {^(?:.{0,200})?}]
    }

    if { $ci } {
        set re "(?i)$re"
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
