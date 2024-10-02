# Copyright Jerily LTD. All Rights Reserved.
# SPDX-FileCopyrightText: 2024 Neofytos Dimitriou (neo@jerily.cy)
# SPDX-License-Identifier: MIT.

lappend auto_path [file join [file dirname [info script]] tcllib]

package require yaml

set data [::yaml::yaml2dict -file [lindex $argv 0]]

foreach rec [dict get $data "test_cases"] {
    set result [list]
    foreach key { family brand model major minor patch patch_minor } {
        if { [dict exists $rec $key] && [dict get $rec $key] ne "" } {
            lappend result $key [dict get $rec $key]
        }
    }
    puts [list [dict get $rec "user_agent_string"] $result]
}

