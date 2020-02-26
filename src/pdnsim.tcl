sta::define_cmd_args "analyze_power_grid" { 
  [-vsrc vsrc_file ]\
  [-res_cfg resistance_cfg_file]}

proc analyze_power_grid { args } {
  sta::parse_key_args "analyze_power_grid" args \
    keys {-vsrc -res_cfg} flags {}

  if { [info exists keys(-vsrc)] } {
    set vsrc_file $keys(-vsrc)
    if { [file readable $vsrc_file] } {
      pdnsim_import_vsrc_cfg_cmd $vsrc_file
    } else {
      puts "Warning: cannot read $vsrc_file"
    }
  } else {
    puts "Error: key vsrc not defined."
  }

  if { [info exists keys(-res_cfg)] } {
    set resistance_cfg_file $keys(-res_cfg)
    if { [file readable $resistance_cfg_file] } {
      pdnsim_import_resistance_cfg_cmd $resistance_cfg_file
    } else {
      puts "Warning: cannot read $resistance_cfg_file"
    }
  } else {
    puts "Error: key res_cfg not defined."
  }

  pdnsim_analyze_power_grid_cmd 

}

