%module pdnsim

%{
#include "openroad/OpenRoad.hh"
#include "pdnsim/pdnsim.h"

namespace ord {
pdnsim::PDNSim*
getPDNSim();
}

using ord::getPDNSim;
using pdnsim::PDNSim;
%}

%inline %{

void 
pdnsim_import_vsrc_cfg_cmd(std::string vsrc)
{
  PDNSim* pdnsim = getPDNSim();
  pdnsim->import_vsrc_cfg(vsrc);
}

void 
pdnsim_import_resistance_cfg_cmd(std::string res_cfg)
{
  PDNSim* pdnsim = getPDNSim();
  pdnsim->import_resistance_cfg(res_cfg);
}

void 
pdnsim_analyze_power_grid_cmd()
{
  PDNSim* pdnsim = getPDNSim();
  pdnsim->analyze_power_grid();
}

%} // inline

