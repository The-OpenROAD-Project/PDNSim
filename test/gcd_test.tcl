OpenIRA ira
ira import_lef  ../test/nangate45/Nangate45.lef
ira import_def ../test/gcd/gcd.def
ira import_verilog ../test/gcd/gcd.v
ira set_top_module gcd
ira import_lib ../test/nangate45/NangateOpenCellLibrary_typical.lib
ira import_sdc ../test/gcd/gcd.sdc
ira read_voltage_src ../test/gcd/Vsrc.loc
ira analyze_power_grid 
OpenIRA
