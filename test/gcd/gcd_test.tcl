PDNSim pdnsim
pdnsim import_lef  ./PDNSim/test/nangate45/Nangate45.lef
pdnsim import_def ./PDNSim/test/gcd/gcd.def
pdnsim import_verilog ./PDNSim/test/gcd/gcd.v
pdnsim set_top_module gcd
pdnsim import_lib ./PDNSim/test/nangate45/NangateOpenCellLibrary_typical.lib
pdnsim import_sdc ./PDNSim/test/gcd/gcd.sdc
pdnsim read_voltage_src ./PDNSim/test/gcd/Vsrc.loc
pdnsim analyze_power_grid 
