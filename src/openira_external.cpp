#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "openira_external.h"
#include "db.h"
#include "lefin.h"
#include "defin.h"
#include "gmat.h"
#include "ir_solver.h"
#include "node.h"

using odb::dbDatabase;
using namespace std;
using std::vector;

IRSolverExternal::
IRSolverExternal() : 
  db_id(INT_MAX) {
};

IRSolverExternal::
~IRSolverExternal() {};

void
IRSolverExternal::help() {
cout <<endl;
cout << "import_lef [file_name]" << endl;
cout << "    *.lef location " << endl;
cout << "    (Multiple lef files supported. " << endl;
cout << "    Technology LEF must be ahead of other LEFs.) " << endl;
cout << endl; 
cout << "import_def [file_name]" << endl;
cout << "    *.def location " << endl;
cout << "    (Required due to FloorPlan information)" << endl;
cout << endl; 
cout <<"import_db [file_name]" << endl;
cout <<"     Input DB file location" << endl;
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
cout << "analyze_power_grid" << endl;
cout << "    Solver for IR drop on VSS net and layer 1" << endl;
cout << endl; 
}

void IRSolverExternal::import_lef(const char* lef){ 
  if( db_id == INT_MAX ) {
    db = odb::dbDatabase::create();
    db_id = db->getId();
  }
  else {
    db = odb::dbDatabase::getDatabase(db_id);
  }
  odb::lefin lefReader(db, false);
  lefReader.createTechAndLib("testlib", lef);
}

void IRSolverExternal::import_def(const char* def){
  if( db_id == INT_MAX ) {
    db = odb::dbDatabase::create();
    db_id = db->getId();
  }
  else {
    db = odb::dbDatabase::getDatabase(db_id);
  }
  odb::defin defReader(db);

  std::vector<odb::dbLib *> search_libs;
  odb::dbSet<odb::dbLib> libs = db->getLibs();
  odb::dbSet<odb::dbLib>::iterator itr;
  for( itr = libs.begin(); itr != libs.end(); ++itr ) {
    search_libs.push_back(*itr);
  }
  odb::dbChip* chip = defReader.createChip( search_libs,  def );
}

void 
IRSolverExternal::import_sdc(const char* sdc) {
  sdc_file = sdc;
  cout<< "INFO: Reading SDC file " << sdc_file << endl;
}

void 
IRSolverExternal::set_top_module(const char* topCellName) {
  top_cell_name = topCellName;
  cout<< "INFO: Top module set " << top_cell_name << endl;
}


void
IRSolverExternal::import_verilog(const char* verilog) {
  verilog_stor = verilog;//.push_back(verilog);
  cout<< "INFO: Reading Verilog file " << verilog_stor << endl;
}

void 
IRSolverExternal::import_lib(const char* lib){
  lib_stor.push_back(lib);
}

void
IRSolverExternal::read_voltage_src(const char* vsrc) {
  vsrc_loc = vsrc;
  cout<< "INFO: Reading Voltage source file " << vsrc_loc << endl;
}

void IRSolverExternal::import_db(const char* dbLoc) {
      if( db_id == INT_MAX ) {
        db = odb::dbDatabase::create();
        db_id = db->getId();
        FILE* fp = fopen(dbLoc, "rb");
        if( fp == NULL ) {
        std::cout << "ERROR: Can't open " <<  dbLoc << endl;
        exit(1);
        }
        db->read(fp);
        fclose(fp);
       }
       else {
         db = odb::dbDatabase::getDatabase(db_id);
       }

}

void IRSolverExternal::analyze_power_grid() {
    GMat* gmat_obj;
    IRSolver* irsolve_h = new IRSolver(db, verilog_stor,top_cell_name, sdc_file, lib_stor, vsrc_loc);
    gmat_obj = irsolve_h->GetGMat();
    irsolve_h->solve_ir();
    std::vector<Node*> nodes = gmat_obj->getNodes();    
    int unit_micron = (db->getTech())->getDbUnitsPerMicron();

    ofstream current_file;
    ofstream voltage_file;
    int vsize;
    current_file.open ("J.csv");
    voltage_file.open ("V.csv");
    vsize = nodes.size();
    for (int n=0; n<vsize; n++)
    {
        Node* node = nodes[n];
        if(node->GetLayerNum() !=1)
            continue;
        NodeLoc loc = node->GetLoc();
        current_file << double(loc.first)/unit_micron<<","<<double(loc.second)/unit_micron<<","<< std::setprecision(10)<<node->getCurrent() <<"\n";
        voltage_file << double(loc.first)/unit_micron<<","<<double(loc.second)/unit_micron<<","<< std::setprecision(10)<<node->getVoltage() <<"\n";
    }
    cout << "\n" <<endl;
    cout << "######################################" <<endl;
    cout<< "Worstcase Voltage: " << irsolve_h->wc_voltage << endl;
    cout<< "Average IR drop  : " << irsolve_h->vdd - irsolve_h->avg_voltage << endl;
    cout<< "Worstcase IR drop: " << irsolve_h->vdd - irsolve_h->wc_voltage << endl;
    cout << "######################################" <<endl;
    current_file.close();
    voltage_file.close();
}


