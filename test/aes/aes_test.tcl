PDNSim pdnsim
pdnsim import_lef  ./PDNSim/test/nangate45/Nangate45.lef
pdnsim import_def ./PDNSim/test/aes/aes.def
pdnsim import_verilog ./PDNSim/test/aes/aes.v
pdnsim set_top_module aes_cipher_top
pdnsim import_lib ./PDNSim/test/nangate45/NangateOpenCellLibrary_typical.lib
pdnsim import_sdc ./PDNSim/test/aes/aes.sdc
pdnsim read_voltage_src ./PDNSim/test/aes/Vsrc.loc
pdnsim read_default_resistance ./PDNSim/test/nangate45/default_resistance.cfg
pdnsim analyze_power_grid 
