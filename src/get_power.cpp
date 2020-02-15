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
#include "get_power.h"
#include "util.h"
#include <iostream>
#include <tcl.h>
namespace sta {
class Sta;
}

using namespace sta;
using namespace std;
using std::vector;

vector<pair<string, double>> PowerInst::executePowerPerInst(
    string         topCellName,
    string         verilogName,
    vector<string> libStor,
    string         sdcName)
{
  std::cout << "\n" << endl;
  std::cout << "INFO: Executing STA for Power" << endl;
  std::cout << "INFO: Execute STA" << endl;
  std::cout << "INFO: Files for STA" << endl;
  std::cout << "Verilog      : " << verilogName << endl;
  std::cout << "topCellName  : " << topCellName << endl;
  // cout << "spefFileName" << spefFile << endl;
  for (auto& libName : libStor) {
    cout << "Liberty      : " << libName << endl;
  }
  cout << "SDCName      : " << sdcName << endl << endl;

  // STA object create
  _sta = new Sta;

  // Tcl Interpreter settings
  //    Tcl_FindExecutable(argv[0]);
  _interp = Tcl_CreateInterp();

  // Initialize the TCL interpreter
  Tcl_Init(_interp);

  // define swig commands
  Sta_Init(_interp);

  // load encoded TCL functions
  evalTclInit(_interp, tcl_inits);
  // initialize TCL commands
  Tcl_Eval(_interp, "sta::show_splash");
  Tcl_Eval(_interp, "namespace import sta::*");

  Tcl_Eval(_interp, "define_sta_cmds");

  // initialize STA objects
  initSta();
  Sta::setSta(_sta);
  _sta->makeComponents();
  _sta->setTclInterp(_interp);

  // environment settings

  string cornerName = "wst";
  // string cornerNameFF="bst";

  StringSet cornerNameSet;
  cornerNameSet.insert(cornerName.c_str());

  // define_corners
  _sta->makeCorners(&cornerNameSet);
  Corner* corner = _sta->findCorner(cornerName.c_str());

  // read_liberty
  for (auto& libName : libStor) {
    _sta->readLiberty(libName.c_str(), corner, MinMaxAll::max(), false);
  }

  // read_netlist
  NetworkReader* networkReader = _sta->networkReader();
  if (!networkReader) {
    cout << "ERROR: Internal OpenSTA has problem for generating networkReader"
         << endl;
    exit(1);
  }

  // Parsing the Verilog
  _sta->readNetlistBefore();
  if (!readVerilogFile(verilogName.c_str(), _sta->networkReader())) {
    cout << "ERROR: OpenSTA failed to read Verilog file!" << endl;
    exit(1);
  }

  // link_design
  Tcl_Eval(_interp, string("set link_make_black_boxes 0").c_str());
  Tcl_Eval(_interp, string("link_design " + topCellName).c_str());

  bool is_linked = networkReader->isLinked();
  if (is_linked) {
    cout << "INFO: Design in linked" << endl;
  } else {
    cout << "ERROR:  Linking Fail. Please put liberty files ";
    cout << "to instantiate OpenSTA correctly" << endl;
    exit(1);
  }

  // read_parasitics
  // bool parasitics =
  //    _sta->readParasitics(spefFile.c_str(),
  //            _sta->currentInstance(),
  //            MinMaxAll::max(), false, true, 0.0,
  //            reduce_parasitics_to_pi_elmore, false, true, true);

  // if(isClockGiven) {
  //    GenerateClockSta();
  //}
  // else {
  Tcl_Eval(_interp, string("sta::read_sdc " + sdcName).c_str());
  //}

  PowerInst::UpdateTimingSta();

  // UpdateNetWeightSta();

  vector<pair<string, double>> power_report;

  sta::Network* network = _sta->network();
  sta::Power*   power   = _sta->power();
  PowerResult   total, sequential, combinational, macro, pad;
  power->power(corner,
               total,
               sequential,
               combinational,
               macro,
               pad);  // TODO called for preamble
  LeafInstanceIterator* inst_iter = network->leafInstanceIterator();
  PowerResult           total_calc;
  total_calc.clear();
  while (inst_iter->hasNext()) {
    Instance*    inst = inst_iter->next();
    LibertyCell* cell = network->libertyCell(inst);
    if (cell) {
      PowerResult inst_power;
      power->power(inst, corner, inst_power);
      total_calc.incr(inst_power);
      power_report.push_back(
          make_pair(string(network->name(inst)), inst_power.total()));
      // cout << string(network->name(inst)) << inst_power.total() << endl;
    }
  }
  delete inst_iter;

  // cout <<"Total power:" << total.total() << endl;
  // cout <<"Total power calculated:" << total_calc.total() << endl;
  return power_report;
}

void PowerInst::UpdateTimingSta()
{
  _sta->updateTiming(true);
}
