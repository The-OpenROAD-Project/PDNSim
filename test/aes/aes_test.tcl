PDNSim pdnsim
pdnsim import_lef  ../test/nangate45/Nangate45.lef
pdnsim import_def ../test/aes/aes.def
pdnsim import_verilog ../test/aes/aes.v
pdnsim set_top_module aes
pdnsim import_lib ../test/nangate45/NangateOpenCellLibrary_typical.lib
pdnsim import_sdc ../test/aes/aes.sdc
pdnsim read_voltage_src ../test/aes/Vsrc.loc
pdnsim analyze_power_grid 
