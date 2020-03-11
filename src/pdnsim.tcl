sta::define_cmd_args "analyze_power_grid" { 
  [-vsrc vsrc_file ]
  [-outfile out_file]}

proc analyze_power_grid { args } {
  sta::parse_key_args "analyze_power_grid" args \
    keys {-vsrc -outfile} flags {}

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

  if { [info exists keys(-outfile)] } {
    set out_file $keys(-outfile)
     pdnsim_import_out_file_cmd $out_file
  }
  pdnsim_analyze_power_grid_cmd 
}

sta::define_cmd_args "check_power_grid" { 
  [-vsrc vsrc_file ]}

proc check_power_grid { args } {
  sta::parse_key_args "check_power_grid" args \
    keys {-vsrc} flags {}

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

  set res [pdnsim_check_connectivity_cmd]
  return $res
}


