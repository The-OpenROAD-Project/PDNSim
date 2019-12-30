#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "db.h"
#include "ir_solver.h"
#include "gmat.h"

using odb::dbDatabase;


using namespace std;
using std::vector;

dbDatabase*  import_db(const char* dbLoc) {
    //  odb::dbDatabase* db = NULL;
      dbDatabase* db_h = new dbDatabase();
      dbDatabase* db = db_h->create();
    
      FILE* fp = fopen(dbLoc, "rb");
      if( fp == NULL ) {
        std::cout << "ERROR: Can't open " <<  dbLoc << endl;
        exit(1);
      }
      db->read(fp);
      fclose(fp);
      int db_id = db->getId(); 
      return db;
    }


int main() {
    GMat* gmat_obj;
    //dbDatabase* db = import_db("/home/sachin00/chhab011/PDNA/aes_pdn.db");
    //dbDatabase* db = import_db("/project/parhi-group00/unnik005/temp/PDNA/aes_pdn.db");
    dbDatabase* db = import_db("/home/sachin00/chhab011/PDN.db");
    cout<< "here2" << endl;
    IRSolver* irsolve_h = new IRSolver(db);
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

