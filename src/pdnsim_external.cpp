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
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "pdnsim_external.h"
#include "db.h"
#include "lefin.h"
#include "defin.h"
#include "gmat.h"
#include "ir_solver.h"
#include "node.h"

using odb::dbDatabase;
using namespace std;
using std::vector;

PDNSim::PDNSim() : db_id(INT_MAX){};

PDNSim::~PDNSim(){};

void PDNSim::help()
{
  cout << endl;
  cout << "=== Populate the DB commands ===" << endl;
  cout << "import_lef [file_name]" << endl;
  cout << "    *.lef location " << endl;
  cout << "    (Multiple lef files supported. " << endl;
  cout << "    Technology LEF must be ahead of other LEFs.) " << endl;
  cout << endl;
  cout << "import_def [file_name]" << endl;
  cout << "    *.def location " << endl;
  cout << "    (Required due to FloorPlan information)" << endl;
  cout << endl;
  cout << "import_db [file_name]" << endl;
  cout << "     Input DB file location" << endl;
  cout << endl;
  cout << "==== Get power commands ====" << endl;
  cout << "import_lib [file_name]" << endl;
  cout << "    *.lib location " << endl;
  cout << "    (Multiple lib files supported. Required for OpenSTA)" << endl;
  cout << endl;
  cout << "import_sdc [file_name]" << endl;
  cout << "    *.sdc location (Required for OpenSTA). " << endl;
  cout << "    SDC: Synopsys Design Constraint (SDC)" << endl;
  cout << endl;
  cout << "import_verilog [file_name]" << endl;
  cout << "    *.v location (Required for OpenSTA)" << endl;
  cout << endl;
  cout << "==== Analysis command  ==== " << endl;
  cout << "read_voltage_src" << endl;
  cout << "   Voltage source location file" << endl;
  cout << "read_default_resistance" << endl;
  cout << "   Read the default resistance file if per-unit R not in DB" << endl;
  cout << "analyze_power_grid" << endl;
  cout << "   Solver for IR drop on VSS net and layer 1" << endl;
  cout << endl;
}

void PDNSim::import_lef(const char* lef)
{
  if (db_id == INT_MAX) {
    db    = odb::dbDatabase::create();
    db_id = db->getId();
  } else {
    db = odb::dbDatabase::getDatabase(db_id);
  }
  odb::lefin lefReader(db, false);
  lefReader.createTechAndLib("testlib", lef);
}

void PDNSim::import_def(const char* def)
{
  if (db_id == INT_MAX) {
    db    = odb::dbDatabase::create();
    db_id = db->getId();
  } else {
    db = odb::dbDatabase::getDatabase(db_id);
  }
  odb::defin defReader(db);

  std::vector<odb::dbLib*>         search_libs;
  odb::dbSet<odb::dbLib>           libs = db->getLibs();
  odb::dbSet<odb::dbLib>::iterator itr;
  for (itr = libs.begin(); itr != libs.end(); ++itr) {
    search_libs.push_back(*itr);
  }
  odb::dbChip* chip = defReader.createChip(search_libs, def);
}

void PDNSim::import_sdc(const char* sdc)
{
  sdc_file = sdc;
  cout << "INFO: Reading SDC file " << sdc_file << endl;
}

void PDNSim::set_top_module(const char* topCellName)
{
  top_cell_name = topCellName;
  cout << "INFO: Top module set " << top_cell_name << endl;
}

void PDNSim::import_verilog(const char* verilog)
{
  cout << "INFO: Reading Verilog file " << verilog_stor << endl;
  verilog_stor = verilog;  //.push_back(verilog);
  cout << "INFO: Reading Verilog file " << verilog_stor << endl;
}

void PDNSim::import_lib(const char* lib)
{
  lib_stor.push_back(lib);
}

void PDNSim::read_voltage_src(const char* vsrc)
{
  vsrc_loc = vsrc;
  cout << "INFO: Reading Voltage source file " << vsrc_loc << endl;
}

void PDNSim::read_default_resistance(const char* def_res)
{
  def_res_val = def_res;
  cout << "INFO: Reading default resistance values " << def_res << endl;
}

void PDNSim::import_db(const char* dbLoc)
{
  if (db_id == INT_MAX) {
    db       = odb::dbDatabase::create();
    db_id    = db->getId();
    FILE* fp = fopen(dbLoc, "rb");
    if (fp == NULL) {
      std::cout << "ERROR: Can't open " << dbLoc << endl;
      exit(1);
    }
    db->read(fp);
    fclose(fp);
  } else {
    db = odb::dbDatabase::getDatabase(db_id);
  }
}

void PDNSim::analyze_power_grid()
{
  GMat*     gmat_obj;
  IRSolver* irsolve_h = new IRSolver(
      db, verilog_stor, top_cell_name, sdc_file, lib_stor, vsrc_loc, def_res_val);
  gmat_obj = irsolve_h->GetGMat();
  irsolve_h->SolveIR();
  std::vector<Node*> nodes       = gmat_obj->GetAllNodes();
  int                unit_micron = (db->getTech())->getDbUnitsPerMicron();
  ofstream current_file;
  ofstream voltage_file;
  current_file.open("J.csv");
  voltage_file.open("V.csv");
  int      vsize;
  vsize = nodes.size();
  for (int n = 0; n < vsize; n++) {
    Node* node = nodes[n];
    if (node->GetLayerNum() != 1)
      continue;
    NodeLoc loc = node->GetLoc();
    current_file << double(loc.first) / unit_micron << ","
                 << double(loc.second) / unit_micron << ","
                 << std::setprecision(10) << node->GetCurrent() << "\n";
    voltage_file << double(loc.first) / unit_micron << ","
                 << double(loc.second) / unit_micron << ","
                 << std::setprecision(10) << node->GetVoltage() << "\n";
  }
  cout << "\n" << endl;
  cout << "######################################" << endl;
  cout << "Worstcase Voltage: " << std::setprecision(5) << irsolve_h->wc_voltage << endl;
  cout << "Average IR drop  : " << std::setprecision(5) << irsolve_h->vdd - irsolve_h->avg_voltage
       << endl;
  cout << "Worstcase IR drop: " << std::setprecision(5) << irsolve_h->vdd - irsolve_h->wc_voltage
       << endl;
  cout << "######################################" << endl;
  current_file.close();
  voltage_file.close();
}
