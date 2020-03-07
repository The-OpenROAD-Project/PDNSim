read_lef  Nangate45.lef
read_def gcd.def
read_liberty NangateOpenCellLibrary_typical.lib
read_sdc gcd.sdc
report_checks


#set res [check_power_grid -vsrc Vsrc_gcd.loc ]
#if { $res == 1} {
# puts "Connectivity check passed"
#} else {
# puts "ERROR: Connectivity check failed"
#}
analyze_power_grid -vsrc Vsrc_gcd.loc 
