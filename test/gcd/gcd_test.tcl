PDNSim pdnsim
pdnsim import_lef  ../test/nangate45/Nangate45.lef
pdnsim import_def ../test/gcd/gcd.def
pdnsim import_verilog ../test/gcd/gcd.v
pdnsim set_top_module gcd
pdnsim import_lib ../test/nangate45/NangateOpenCellLibrary_typical.lib
pdnsim import_sdc ../test/gcd/gcd.sdc
pdnsim read_voltage_src ../test/gcd/Vsrc.loc
pdnsim analyze_power_grid 
