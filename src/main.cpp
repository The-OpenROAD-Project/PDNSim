#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "main.h"
#include "db.h"
#include "lefin.h"
#include "defin.h"
#include "ir_solver.h"
#include "gmat.h"

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
}

void
IRSolverExternal::import_verilog(const char* verilog) {
  verilog_stor.push_back(verilog);
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

    //dbDatabase* db = import_db("/home/sachin00/chhab011/PDNA/aes_pdn.db");
    IRSolverExternal*  ir_obj = new IRSolverExternal();
    ir_obj->import_lef("/home/sachin00/chhab011/OpeNPDN/platforms/nangate45/NangateOpenCellLibrary.mod.lef");
    ir_obj->import_def("/home/sachin00/chhab011/PDNA_clean/gcd_pdn.def");
    //ir_obj->import_db("/home/sachin00/chhab011/PDN.db");
    cout<< "here2" << endl;
    IRSolver* irsolve_h = new IRSolver(ir_obj->db);
    gmat_obj = irsolve_h->GetGMat();
    gmat_obj->print();
    ofstream myfile;
    int vsize;
    vector<double> test_J = irsolve_h->getJ();//need a copy as superlu destros that loc
    myfile.open ("J.csv");
    vsize = test_J.size();
    for (int n=0; n<vsize; n++)
    {
        myfile <<  std::setprecision(10)<<test_J[n] <<"\n";
    }

    myfile.close();
    irsolve_h->solve_ir();
    return 1;

}

