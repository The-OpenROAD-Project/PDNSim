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
  set net "VDD"
  pdnsim_set_power_net $net
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
  [-vsrc vsrc_file ]
  [-vdd]
  [-vss]}

proc check_power_grid { args } {
  sta::parse_key_args "check_power_grid" args \
    keys {} flags {-vdd -vss}

  if { [info exists keys(-vsrc)] } {
    set net "VDD"
    set vsrc_file $keys(-vsrc)
    if { [file readable $vsrc_file] } {
      pdnsim_import_vsrc_cfg_cmd $vsrc_file
    } else {
      ord::error "Cannot read $vsrc_file"
    }
  } 

  set net ""
  if { [info exists flags(-vdd)] } {
    set net "VDD"
    pdnsim_set_power_net $net
  } elseif {[info exists flags(-vss)]} {
    set net "VSS"
    pdnsim_set_power_net $net
  } else {
    ord::error "Please specify either power (-vdd) or ground (-vss) option."
  }
  if { [ord::db_has_rows] } {
  	set res [pdnsim_check_connectivity_cmd]
  	return $res
  } else {
  	ord::error "No rows defined in design. Use initialize_floorplan to add rows" 
  }
}

sta::define_cmd_args "write_pg_spice" { 
  [-vsrc vsrc_file ]
  [-outfile out_file]}

proc write_pg_spice { args } {
  sta::parse_key_args "write_pg_spice" args \
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
     pdnsim_import_spice_out_file_cmd $out_file
  }
  set net "VDD"
  pdnsim_set_power_net $net
  if { [ord::db_has_rows] } {
    pdnsim_write_pg_spice_cmd
  } else {
  	ord::error "no rows defined in design. Use initialize_floorplan to add rows" 
  }
}
