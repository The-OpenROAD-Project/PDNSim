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

void PdnsimLogger(pdnsim_status status, int message_id, string message) {
  switch(status){
    case INFO: cout<<"[INFO]";
          break;
    case WARN: cout<<"[WARN]";
          break;
    case ERROR: cout<<"[ERROR]";
          break;
    case CRIT: cout<<"[CRIT]";
          break;
    default: cout<<"[ERROR] "<<status;
  }
  if(message_id > 0) {
    cout<<"[PSIM-"<< setfill('0') << setw(4) << message_id <<"]";
  }
  cout<<message<<endl;
}

PDNSim::PDNSim()
  : _db(nullptr),
  _sta(nullptr),
  _vsrc_loc(""),
  _power_net(""),
  _out_file(""),
  _spice_out_file(""){
};

PDNSim::~PDNSim() {
  _db = nullptr;
  _sta = nullptr; 
  _vsrc_loc = "";
  _power_net = "";
  _out_file = "";
  _spice_out_file = "";
}

void PDNSim::setDb(odb::dbDatabase* db){
  _db = db;
}
void PDNSim::setSta(sta::dbSta* sta){
  _sta = sta;
}

void PDNSim::set_power_net(std::string net){
  _power_net= net;
}


void PDNSim::import_vsrc_cfg(std::string vsrc)
{
  _vsrc_loc = vsrc;
  string s = "Reading Voltage source file " + _vsrc_loc;
  PdnsimLogger(INFO,-1,s);
}

void PDNSim::import_out_file(std::string out_file)
{
  _out_file = out_file;
  string s = "Output voltage file specified " + _out_file;
  PdnsimLogger(INFO,-1,s);
}

void PDNSim::import_spice_out_file(std::string out_file)
{
  _spice_out_file = out_file;
  string s = "Output spice file specified " + _spice_out_file;
  PdnsimLogger(INFO,-1,s);
}

void PDNSim::write_pg_spice() {
  IRSolver* irsolve_h = new IRSolver( _db, _sta, _vsrc_loc, _power_net, _out_file, _spice_out_file);
 
  if(!irsolve_h->Build()){
    string s = "IR solver build unsuccessful";
    PdnsimLogger(ERROR,1,s);
  	delete irsolve_h;
  } else {
    int check_spice = irsolve_h->PrintSpice();
    if(check_spice){
    	string s = "Spice file written: " + _spice_out_file;
      PdnsimLogger(INFO,-1,s);
    }
    else {
      string s = "Spice file not written";
      PdnsimLogger(ERROR,2,s);
    }
  }
}

int PDNSim::analyze_power_grid(){
  GMat*     gmat_obj;
  IRSolver* irsolve_h = new IRSolver( _db, _sta, _vsrc_loc,_power_net, _out_file, _spice_out_file);
  
  if(!irsolve_h->Build()){
    string s = "IR solver build unsuccessful";
    PdnsimLogger(ERROR,1,s);
  	delete irsolve_h;
  	return 0;
  }
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
  //cout << "\n" << endl;
  //cout << "######################################" << endl;
  ostringstream s1,s2,s3;
  s1 << "Worstcase Voltage: " << std::setprecision(5) << irsolve_h->wc_voltage;
  PdnsimLogger(INFO,11,s1.str());
  s2 << "Average IR drop  : " << std::setprecision(5) << irsolve_h->vdd - irsolve_h->avg_voltage;
  PdnsimLogger(INFO,11,s2.str());
  s3 << "Worstcase IR drop: " << std::setprecision(5) << irsolve_h->vdd - irsolve_h->wc_voltage;
  PdnsimLogger(INFO,11,s3.str());
  //cout << "######################################" << endl;
  delete irsolve_h;
  return 1;
}

int PDNSim::check_connectivity() {
  IRSolver* irsolve_h = new IRSolver( _db, _sta, _vsrc_loc, _power_net,_out_file, _spice_out_file);
  if(!irsolve_h->BuildConnection()){
    string s = "IR solver build unsuccessful";
    PdnsimLogger(ERROR,1,s);
  	delete irsolve_h;
  	return 0;
  }
  int val = irsolve_h->GetConnectionTest();
  delete irsolve_h;
  return val;
}

}



