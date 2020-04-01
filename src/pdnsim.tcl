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
      ord::error "Cannot read $vsrc_file"
    }
  } else {
    ord::error "key vsrc not defined."
  }

  if { [info exists keys(-outfile)] } {
    set out_file $keys(-outfile)
     pdnsim_import_out_file_cmd $out_file
  }
  if { [ord::db_has_rows] } {
    pdnsim_analyze_power_grid_cmd
  } else {
  	ord::error "no rows defined in design. Use initialize_floorplan to add rows" 
  }
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
      ord::error "Cannot read $vsrc_file"
    }
  } else {
      ord::error "key vsrc not defined."
  }

  if { [ord::db_has_rows] } {
  	set res [pdnsim_check_connectivity_cmd]
  	return $res
  } else {
  	ord::error "no rows defined in design. Use initialize_floorplan to add rows" 
  }
}


