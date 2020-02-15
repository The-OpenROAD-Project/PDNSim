/*
BSD 3-Clause License

Copyright (c) 2020, The Regents of the University of Minnesota

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "pdnsim_external.h"
#include <tcl.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "db.h"
#include "ir_solver.h"
#include <string>
#include <vector>
#include "gmat.h"
#include "node.h"
#include "parameters.h"

using namespace std;

extern "C" {
int Irsolver_Init(Tcl_Interp* interp);
}

int PDNSimTclAppInit(Tcl_Interp* interp)
{
  std::cout << " > Running PDNSim in interactive mode.\n";

  Tcl_Init(interp);
  Irsolver_Init(interp);

  return TCL_OK;
}

int pdn_sim(Parameters* parmsToPDNSim)
{
  GMat*    gmat_obj;
  PDNSim* ir_obj = new PDNSim();
  ir_obj->import_lef(parmsToPDNSim->getInputLefFile().c_str());
  ir_obj->import_def(parmsToPDNSim->getInputDefFile().c_str());
  ir_obj->import_verilog(parmsToPDNSim->getInputVerilogFile().c_str());
  ir_obj->set_top_module(parmsToPDNSim->getTopModule().c_str());
  ir_obj->import_lib(parmsToPDNSim->getInputLibFile().c_str());
  ir_obj->import_sdc(parmsToPDNSim->getInputSDCFile().c_str());
  ir_obj->read_voltage_src(parmsToPDNSim->getInputVsrcFile().c_str());
  cout << parmsToPDNSim->getInputVsrcFile().c_str() << endl;
  IRSolver* irsolve_h = new IRSolver(ir_obj->db,
                                     ir_obj->verilog_stor,
                                     ir_obj->top_cell_name,
                                     ir_obj->sdc_file,
                                     ir_obj->lib_stor,
                                     ir_obj->vsrc_loc);
  gmat_obj            = irsolve_h->GetGMat();
  irsolve_h->solve_ir();
  std::vector<Node*> nodes = gmat_obj->getNodes();
  int unit_micron          = ((ir_obj->db)->getTech())->getDbUnitsPerMicron();

  ofstream current_file;
  ofstream voltage_file;
  int      vsize;
  current_file.open("J.csv");
  voltage_file.open("V.csv");
  vsize = nodes.size();
  for (int n = 0; n < vsize; n++) {
    // myfile <<  std::setprecision(10)<<test_J[n] <<"\n";
    Node* node = nodes[n];
    if (node->GetLayerNum() != 1)
      continue;
    NodeLoc loc = node->GetLoc();
    current_file << double(loc.first) / unit_micron << ","
                 << double(loc.second) / unit_micron << ","
                 << std::setprecision(10) << node->getCurrent() << "\n";
    voltage_file << double(loc.first) / unit_micron << ","
                 << double(loc.second) / unit_micron << ","
                 << std::setprecision(10) << node->getVoltage() << "\n";
  }
  cout << "\n" << endl;
  cout << "######################################" << endl;
  cout << "Worstcase Voltage: " << irsolve_h->wc_voltage << endl;
  cout << "Average  IR drop : " << irsolve_h->vdd - irsolve_h->avg_voltage
       << endl;
  cout << "Worstcase IR drop: " << irsolve_h->vdd - irsolve_h->wc_voltage
       << endl;
  cout << "######################################" << endl;

  current_file.close();
  voltage_file.close();

  return 1;
}

int main(int argc, char** argv)
{
  std::cout << " ######################################\n";
  std::cout << " # PDNSim: OpenROAD IR analysis tool #\n";
  std::cout << " #       University of Minnesota      #\n";
  std::cout << " # Author:                            #\n";
  std::cout << " #    Vidya Chhabria (UMN)            #\n";
  std::cout << " #    Sachin Sapatnekar (UMN)         #\n";
  std::cout << " ######################################\n";
  std::cout << "\n";

  Tcl_Interp* interp;

  Parameters* parmsToPDNSim = new Parameters(argc, argv);

  if (parmsToPDNSim->isInteractiveMode()) {
    Tcl_Main(argc, argv, PDNSimTclAppInit);
  } else {
    pdn_sim(parmsToPDNSim);
  }

  return 0;
}
