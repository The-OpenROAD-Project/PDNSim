%module pdnsim

%{
#include "openroad/OpenRoad.hh"
#include "pdnsim/PDNSim.h"

namespace ord {
pdnsim::PDNSim*
getPDNSim();
}

using ord::getPDNSim;
using pdnsim::PDNSim;
%}

%inline %{

void 
pdnsim_import_vsrc_cfg_cmd()
{
  PDNSim* pdnsim = getPDNSim();
  pdnsim->import_vsrc_cfg();
}

void 
pdnsim_import_resistance_cfg_cmd()
{
  PDNSim* pdnsim = getPDNSim();
  pdnsim->import_resistance_cfg();
}

void 
pdnsim_analyze_power_grid_cmd()
{
  PDNSim* pdnsim = getPDNSim();
  pdnsim->analyze_power_grid();
}

%} // inline

%include <stl.i>
%include <typemaps.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_pair.i>
