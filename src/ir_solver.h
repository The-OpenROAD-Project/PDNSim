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
#ifndef __IRSOLVER_IRSOLVER_
#define __IRSOLVER_IRSOLVER_

#include "gmat.h"
#include "db.h"
//! Class for IR solver
/* 
 * Builds the equations GV=J and uses SuperLU 
 * to solve the matrix equations
 */
class IRSolver
{
 public:
  //! Constructor for IRSolver class
  /*
   * This constructor creates an instance of the class using
   * the given inputs.
   */
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
    ReadC4Data();
    CreateGmat();
    CreateJ();
    AddC4Bump();
    m_Gmat->GenerateCSCMatrix();
  }
  //! IRSolver destructor
  ~IRSolver() { delete m_Gmat; }
  //! Worst case voltage at the lowest layer nodes
  double                                      wc_voltage;
  //! Voltage supply
  double                                      vdd;
  //! Average voltage at lowest layer nodes  
  double                                      avg_voltage;
  //! Vector of worstcase voltages in the lowest layers
  std::vector<double>                         wc_volt_layer;
  //! Returns the created G matrix for the design
  GMat*                                       GetGMat();
  //! Returns current map represented as a 1D vector
  std::vector<double>                         GetJ();
  //! Function to solve for IR drop 
  void                                        SolveIR();
  //! Function to get the power value from OpenSTA
  std::vector<std::pair<std::string, double>> GetPower();
 
 private:
  //! Pointer to the Db
  odb::dbDatabase*         m_db;
  //! Verilog file
  std::string              m_verilog_stor;
  //! Liberty files
  std::vector<std::string> m_lib_stor;
  //! SDC file
  std::string              m_sdc_file;
  //! Top module name
  std::string              m_top_module;
  //! Voltage source file
  std::string              m_vsrc_file;
  //! G matrix for voltage 
  GMat*                    m_Gmat;
  //! Node density in the lower most layer to append the current sources
  int m_node_density{2800};  // TODO get from somewhere
  //! Current vector 1D
  std::vector<double>                            m_J;
  //! C4 bump locations and values
  std::vector<std::tuple<int, int, int, double>> m_C4Bumps;
  //! Locations of the C4 bumps in the G matrix
  std::vector<NodeIdx>                           m_C4GLoc;
  //! Function to add C4 bumps to the G matrix
  void                                           AddC4Bump();
  //! Function that parses the Vsrc file
  void                                           ReadC4Data();
  //! Function to create a J vector from the current map
  void                                           CreateJ();
  //! Function to create a G matrix using the nodes
  void                                           CreateGmat();
};
#endif
