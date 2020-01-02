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
#include "ir_solver.h"
#include "gmat.h"
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
  //odb::dbDatabase * db = NULL;
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
 // odb::dbDatabase * db = NULL;
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
  cout<< "SDC file :" << sdc_file << endl;
}

void
IRSolverExternal::import_verilog(const char* verilog) {
  verilog_stor = verilog;//.push_back(verilog);
  cout<< "Verilog file :" << verilog_stor << endl;
}

void 
IRSolverExternal::import_lib(const char* lib){
  lib_stor.push_back(lib);
}


void IRSolverExternal::import_db(const char* dbLoc) {
      //odb::dbDatabase* db = NULL;
      //dbDatabase* db_h = new dbDatabase();
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


int main() {
    GMat* gmat_obj;
    dbDatabase* db_obj;
    //dbDatabase* db = import_db("/home/sachin00/chhab011/PDNA/aes_pdn.db");
    IRSolverExternal*  ir_obj = new IRSolverExternal();
    ir_obj->import_lef("/home/sachin00/chhab011/OpeNPDN/platforms/nangate45/NangateOpenCellLibrary.mod.lef");
    ir_obj->import_def("/home/sachin00/chhab011/PDNA_clean/gcd/3_place.def");
    ir_obj->import_verilog("/home/sachin00/chhab011/PDNA_clean/gcd/2_floorplan.v");
    ir_obj->import_lib("/home/sachin00/chhab011/PDNA_clean/gcd/NangateOpenCellLibrary_typical.lib");
    ir_obj->import_sdc("/home/sachin00/chhab011/PDNA_clean/gcd/2_floorplan.sdc");
    //ir_obj->import_db("/home/sachin00/chhab011/PDN.db");
    cout<< "here2" << endl;
    cout << "Verilog file after here2" << ir_obj->verilog_stor << endl;
    IRSolver* irsolve_h = new IRSolver(ir_obj->db, ir_obj->verilog_stor, ir_obj->sdc_file, ir_obj->lib_stor);
    gmat_obj = irsolve_h->GetGMat();
    gmat_obj->print();
    irsolve_h->solve_ir();
    db_obj = ir_obj->db;
    std::vector<Node*> nodes = gmat_obj->getNodes();    
    int unit_micron = (db_obj->getTech())->getDbUnitsPerMicron();

    ofstream current_file;
    ofstream voltage_file;
    int vsize;
    current_file.open ("J.csv");
    voltage_file.open ("V.csv");
    vsize = nodes.size();
    for (int n=0; n<vsize; n++)
    {
        //myfile <<  std::setprecision(10)<<test_J[n] <<"\n";
        Node* node = nodes[n];
        if(node->GetLayerNum() !=1)
            continue;
        NodeLoc loc = node->GetLoc();
        current_file << double(loc.first)/unit_micron<<","<<double(loc.second)/unit_micron<<","<< std::setprecision(10)<<node->getCurrent() <<"\n";
        voltage_file << double(loc.first)/unit_micron<<","<<double(loc.second)/unit_micron<<","<< std::setprecision(10)<<node->getVoltage() <<"\n";
    }

    current_file.close();
    voltage_file.close();


    return 1;

}
   
