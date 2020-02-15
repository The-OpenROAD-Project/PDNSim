#ifndef __IRSOLVER_EXTERNAL__
#define __IRSOLVER_EXTERNAL__

#include "db.h"
class PDNSim
{
 public:
  PDNSim();
  ~PDNSim();
  odb::dbDatabase*         db = NULL;
  std::string              verilog_stor;
  std::vector<std::string> lib_stor;
  std::string              sdc_file;
  std::string              top_cell_name;
  std::string              vsrc_loc;
  void                     help();
  void                     import_lef(const char* lef);
  void                     import_def(const char* def);
  void                     import_sdc(const char* sdc);
  void                     set_top_module(const char* verilogModule);
  void                     import_verilog(const char* verilog);
  void                     import_lib(const char* lib);
  void                     import_db(const char* dbLoc);
  void                     read_voltage_src(const char* vsrc);
  void                     analyze_power_grid();

 private:
  int db_id;
};
#endif
