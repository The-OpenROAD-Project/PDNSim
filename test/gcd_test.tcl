read_lef  Nangate45.lef
read_def gcd.def
read_liberty NangateOpenCellLibrary_typical.lib
read_sdc gcd.sdc
report_checks
analyze_power_grid -vsrc Vsrc_gcd.loc -res_cfg default_resistance.cfg
