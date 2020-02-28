read_lef  Nangate45.lef
read_def aes.def
read_liberty NangateOpenCellLibrary_typical.lib
read_sdc aes.sdc
report_checks
analyze_power_grid -vsrc Vsrc_aes.loc  -res_cfg default_resistance.cfg
