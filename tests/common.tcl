
tcltest::testConstraint threaded [expr { ![catch { package require Thread }] }]


set uap_test_data_dir [file join [file dirname [info script]] data]
if {[info exists ::env(UAP_TEST_DATA_DIR)]} {
    set uap_test_data_dir $::env(UAP_TEST_DATA_DIR)
}
