#ifndef __IRSOLVER_IRSOLVER_
#define __IRSOLVER_IRSOLVER_

#include "gmat.h"
#include "db.h"

class IRSolver
{
 public:
  IRSolver(odb::dbDatabase*         t_db,
           std::string              verilog_stor,
           std::string              top_module,
           std::string              sdc_file,
           std::vector<std::string> lib_stor,
           std::string              vsrc_loc)
  {
    m_db           = t_db;
    m_verilog_stor = verilog_stor;
    m_sdc_file     = sdc_file;
    m_lib_stor     = lib_stor;
    m_top_module   = top_module;
    m_vsrc_file    = vsrc_loc;
    m_readC4Data();
    m_createGmat();
    m_createJ();
    m_addC4Bump();
    m_Gmat->GenerateCSCMatrix();
  }
  ~IRSolver() { delete m_Gmat; }
  double                                      wc_voltage;
  double                                      vdd;
  double                                      avg_voltage;
  std::vector<double>                         wc_volt_layer;
  GMat*                                       GetGMat();
  std::vector<double>                         getJ();
  void                                        solve_ir();
  std::vector<std::pair<std::string, double>> m_getPower();

 private:
  odb::dbDatabase*         m_db;
  std::string              m_verilog_stor;
  std::vector<std::string> m_lib_stor;
  std::string              m_sdc_file;
  std::string              m_top_module;
  std::string              m_vsrc_file;
  GMat*                    m_Gmat;
  int m_node_density{2800};  // TODO get from somehwere
  std::vector<double>                            m_J;
  std::vector<std::tuple<int, int, int, double>> m_C4Bumps;
  std::vector<NodeIdx>                           m_C4GLoc;
  void                                           m_addC4Bump();
  void                                           m_readC4Data();
  void                                           m_createJ();
  void                                           m_createGmat();
};
#endif
