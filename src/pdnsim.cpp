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
#include "pdnsim/pdnsim.h"
#include <tcl.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "opendb/db.h"
#include "ir_solver.h"
#include <string>
#include <vector>
#include "gmat.h"
#include "node.h"
//#include "parameters.h"

using namespace std;

namespace pdnsim {

using namespace std;

PDNSim::PDNSim()
  : _db(nullptr),
  _sta(nullptr),
  _vsrc_loc(""),
  _res_cfg("") {
};

PDNSim::~PDNSim() {
  _db = nullptr;
  _sta = nullptr;
  _vsrc_loc = "";
  _res_cfg = "";
}

void PDNSim::setDb(odb::dbDatabase* db){
  _db = db;
}
void PDNSim::setSta(sta::dbSta* sta){
  _sta = sta;
}

void PDNSim::import_vsrc_cfg(std::string vsrc)
{
  _vsrc_loc = vsrc;
  cout << "INFO: Reading Voltage source file " << _vsrc_loc << endl;
}

void PDNSim::import_resistance_cfg(std::string res_cfg)
{
  _res_cfg = res_cfg;
  cout << "INFO: Reading default resistance values " << _res_cfg << endl;
}

void PDNSim::analyze_power_grid(){
  GMat*     gmat_obj;
  //IRSolver* irsolve_h = new IRSolver(
  //    db, verilog_stor, top_cell_name, sdc_file, lib_stor, vsrc_loc, def_res_val);
  IRSolver* irsolve_h = new IRSolver( _db, _sta, _vsrc_loc, _res_cfg);
  gmat_obj = irsolve_h->GetGMat();
  irsolve_h->SolveIR();
  std::vector<Node*> nodes       = gmat_obj->GetAllNodes();
  int                unit_micron = (_db->getTech())->getDbUnitsPerMicron();
  int      vsize;
  vsize = nodes.size();
  for (int n = 0; n < vsize; n++) {
    Node* node = nodes[n];
    if (node->GetLayerNum() != 1)
      continue;
    NodeLoc loc = node->GetLoc();
  }
  cout << "\n" << endl;
  cout << "######################################" << endl;
  cout << "Worstcase Voltage: " << std::setprecision(5) << irsolve_h->wc_voltage << endl;
  cout << "Average IR drop  : " << std::setprecision(5) << irsolve_h->vdd - irsolve_h->avg_voltage
       << endl;
  cout << "Worstcase IR drop: " << std::setprecision(5) << irsolve_h->vdd - irsolve_h->wc_voltage
       << endl;
  cout << "######################################" << endl;
}

}



