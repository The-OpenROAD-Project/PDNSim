IRSolverExternal ir
ir import_lef  ../test/nangate45/Nangate45.lef
ir import_def ../test/gcd/gcd.def
ir import_verilog ../test/gcd/gcd.v
ir set_top_module gcd
ir import_lib ../test/nangate45/NangateOpenCellLibrary_typical.lib
ir import_sdc ../test/gcd/gcd.sdc
ir read_voltage_src ../test/gcd/Vsrc.loc
ir analyze_power_grid 
