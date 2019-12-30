#ifndef __IRSOLVER_EXTERNAL__
#define __IRSOLVER_EXTERNAL__

#include "db.h"
class IRSolverExternal {
public:
    IRSolverExternal();
    ~IRSolverExternal();
    odb::dbDatabase* db = NULL;
    void help();
    void import_lef(const char* lef);
    void import_def(const char* def);
    void import_sdc(const char* sdc);
    void import_verilog(const char* verilog);
    void import_lib(const char* lib);
    void import_db(const char* dbLoc);
private:
    int db_id;
    std::vector<std::string> verilog_stor;
    std::vector<std::string> lib_stor;
    std::string net_weight_file;
    std::string sdc_file;

};
#endif

