# PGA

OpenPGA: Open-source static power-grid analyzer.

Inputs

DEF : Placed and PDN synthesized
LEF : Tech and Cell LEF
LIB

Outputs:
Static IR drop reports
Static IR drop maps for VDD and GND networks


Dependencies:
    OpenDB
    OpenSTA
   
Commands in TCL shell:
analyze_pg

Default it runs lower most layer and VSS net

analyze_pg -layer M2 -net VDD
analyze_pg -layer M5 -net VSS


