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
#include <math.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <map>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <sstream>
#include <iterator>
#include <string>

#include "slu_ddefs.h"
#include "db.h"
#include "ir_solver.h"
#include "node.h"
#include "gmat.h"
#include "get_power.h"

using odb::dbBlock;
using odb::dbBox;
using odb::dbChip;
using odb::dbDatabase;
using odb::dbInst;
using odb::dbNet;
using odb::dbSBox;
using odb::dbSet;
using odb::dbSigType;
using odb::dbSWire;
using odb::dbTech;
using odb::dbTechLayer;
using odb::dbTechLayerDir;
using odb::dbVia;

using namespace std;
using std::vector;


//! Returns the created G matrix for the design
/*
 * \return G Matrix
 */
GMat* IRSolver::GetGMat()
{
  return m_Gmat;
}


//! Returns current map represented as a 1D vector
/* 
 * \return J vector
 */
vector<double> IRSolver::GetJ()
{
  return m_J;
}


//! Function to solve for voltage using SuperLU 
void IRSolver::SolveIR()
{
  clock_t t1, t2;
  SuperMatrix       A, L, U, B;
  SuperLUStat_t     stat;
  superlu_options_t options;
  int               nrhs = 1;
  CscMatrix*        Gmat = m_Gmat->GetGMat();
  int               m    = Gmat->num_rows;
  int               n    = Gmat->num_cols;
  int               info;
  int*              perm_r; /* row permutations from partial pivoting */
  int*              perm_c; /* column permutation vector */
  vector<double>    J   = GetJ();
  double*           rhs = &J[0];
  dCreate_Dense_Matrix(&B, m, nrhs, rhs, m, SLU_DN, SLU_D, SLU_GE);
  int     nnz     = Gmat->nnz;
  double* values  = &(Gmat->values[0]);
  int*    row_idx = &(Gmat->row_idx[0]);
  int*    col_ptr = &(Gmat->col_ptr[0]);
  dCreate_CompCol_Matrix(
      &A, m, n, nnz, values, row_idx, col_ptr, SLU_NC, SLU_D, SLU_GE);
  if (!(perm_r = intMalloc(m)))
    ABORT("Malloc fails for perm_r[].");
  if (!(perm_c = intMalloc(n)))
    ABORT("Malloc fails for perm_c[].");
  set_default_options(&options);
  options.ColPerm = COLAMD;
  /* Initialize the statistics variables. */
  StatInit(&stat);
  // dPrint_CompCol_Matrix("A", &A);
  // dPrint_Dense_Matrix("B", &B);
  cout << "\n" << endl;
  t1 = clock();
  cout << "INFO: Solving GV=J" << endl;
  cout << "INFO: SuperLU begin solving" << endl;
  /* Solve the linear system. */
  dgssv(&options, &A, perm_c, perm_r, &L, &U, &B, &stat, &info);
  t2 = clock();
  float time_to_solve;
  time_to_solve = float(t2-t1)/CLOCKS_PER_SEC;
  cout << "INFO: SuperLU finished solving" << endl;
  // dPrint_Dense_Matrix("B", &B);
  DNformat*    Bstore = (DNformat*) B.Store;
  register int i, j, lda = Bstore->lda;
  double*      dp;
  double       volt;
  double       sum_volt = 0;
  int          node_num = 0;
  wc_voltage            = vdd;
  dp                    = (double*) Bstore->nzval;
  int num_nodes         = m_Gmat->GetNumNodes();
  for (j = 0; j < B.ncol; ++j) {
    for (i = 0; i < B.nrow; ++i) {
      if (node_num >= num_nodes) {
        break;
      }
      Node* node = m_Gmat->GetNode(node_num);
      volt       = dp[i + j * lda];
      sum_volt   = sum_volt + volt;
      if (volt < wc_voltage) {
        wc_voltage = volt;
      }
      node->SetVoltage(volt);
      node_num++;
    }
  }
  avg_voltage = sum_volt / num_nodes;
  // TODO keep copies for LU for later?
  /* De-allocate storage */
  // SUPERLU_FREE (rhs);
  SUPERLU_FREE(perm_r);
  SUPERLU_FREE(perm_c);
  Destroy_SuperMatrix_Store(&A);
  Destroy_SuperMatrix_Store(&B);
  Destroy_SuperNode_Matrix(&L);
  Destroy_CompCol_Matrix(&U);
  StatFree(&stat);
}


//! Function to add C4 bumps to the G matrix
void IRSolver::AddC4Bump()
{
  if (m_C4Bumps.size() == 0) {
    cout << "ERROR: Invalid number of voltage sources" << endl;
  }
  for (int it = 0; it < m_C4Bumps.size(); ++it) {
    double  voltage  = get<3>(m_C4Bumps[it]);
    NodeIdx node_loc = m_C4GLoc[it];
    m_Gmat->AddC4Bump(node_loc, it);  // add the 0th bump
    m_J.push_back(voltage);           // push back first vdd
    vdd = voltage;
  }
}



//! Function that parses the Vsrc file
void IRSolver::ReadC4Data()
{
  cout << "Voltage file" << m_vsrc_file << endl;
  cout << "INFO: Reading location of VDD and VSS sources " << endl;
  std::ifstream file(m_vsrc_file);    
  std::string line = "";
  // Iterate through each line and split the content using delimiter
  while (getline(file, line)) {
    tuple<int, int, int, double> c4_bump;
    int                          first, second, layer;
    double                       voltage;
    stringstream                 X(line);
    string                       val;
    for (int i = 0; i < 4; ++i) {
      getline(X, val, ',');
      if (i == 0) {
        first = stoi(val);
      } else if (i == 1) {
        second = stoi(val);
      } else if (i == 2) {
        layer = stoi(val);
      } else {
        voltage = stod(val);
      }
    }
    m_C4Bumps.push_back(make_tuple(first, second, layer, voltage));
  }
  file.close();
}


//! Function that parses the Vsrc file
void IRSolver::ReadResData()
{
  cout << "Default resistance file" << m_def_res << endl;
  cout << "INFO: Reading resistance of layers and vias " << endl;
  std::ifstream file(m_def_res);    
  std::string line = "";
  int line_num = 0;
  // Iterate through each line and split the content using delimiter
  while (getline(file, line)) {
    line_num ++;
    if (line_num == 1) {
      continue;
    }
    //tuple<int, double, double> layer_res;
    int                          routing_level;
    double                       res_per_unit;
    double                       res_via;
    stringstream                 X(line);
    string                       val;
    for (int i = 0; i < 3; ++i) {
      getline(X, val, ',');
      if (i == 0) {
        routing_level = stoi(val);
      } else if (i == 1) {
        res_per_unit = stod(val);
      } else {
        res_via = stod(val);
      }
    }
    m_layer_res.push_back(make_tuple(routing_level, res_per_unit, res_via));
  }
  file.close();
}


//! Function to create a J vector from the current map
void IRSolver::CreateJ()
{  // take current_map as an input?
  int num_nodes = m_Gmat->GetNumNodes();
  m_J.resize(num_nodes, 0);

  vector<pair<string, double>> power_report = GetPower();
  dbChip*                      chip         = m_db->getChip();
  dbBlock*                     block        = chip->getBlock();
  for (vector<pair<string, double>>::iterator it = power_report.begin();
       it != power_report.end();
       ++it) {
    dbInst* inst = block->findInst(it->first.c_str());
    if (inst == NULL) {
      cout << "Warning instance " << it->first << " not found within database"
           << endl;
      continue;
    }
    int x, y;
    inst->getLocation(x, y);
    int   l      = 1;  // atach to the bottom most routing layer
    Node* node_J = m_Gmat->GetNode(x, y, l);
    node_J->AddCurrentSrc(it->second);
  }
  for (int i = 0; i < num_nodes; ++i) {
    Node* node_J = m_Gmat->GetNode(i);
    m_J[i] = -1 * (node_J->GetCurrent());  // as MNA needs negative
    // cout << m_J[i] <<endl;
  }
  cout << "INFO: Created J vector" << endl;
}


//! Function to create a G matrix using the nodes
void IRSolver::CreateGmat()
{
  std::vector<Node*> node_vector;
  dbTech*                      tech   = m_db->getTech();
  dbSet<dbTechLayer>           layers = tech->getLayers();
  dbSet<dbTechLayer>::iterator litr;
  int num_routing_layers = tech->getRoutingLayerCount();

  int num_c4                = m_C4Bumps.size();
  m_Gmat                    = new GMat(num_routing_layers, num_c4);
  dbChip*             chip  = m_db->getChip();
  dbBlock*            block = chip->getBlock();
  dbSet<dbNet>        nets  = block->getNets();
  std::vector<dbNet*> vdd_nets;
  std::vector<dbNet*> gnd_nets;
  int                 netCNT = nets.size();

  dbSet<dbNet>::iterator nIter;
  for (nIter = nets.begin(); nIter != nets.end(); ++nIter) {
    dbNet* curDnet = *nIter;

    dbSigType nType = curDnet->getSigType();
    if (nType == dbSigType::GROUND) {
      gnd_nets.push_back(curDnet);
    } else if (nType == dbSigType::POWER) {
      vdd_nets.push_back(curDnet);
    } else {
      continue;
    }
  }
  std::vector<dbNet*>::iterator vIter;
  for (vIter = vdd_nets.begin(); vIter != vdd_nets.end(); ++vIter) {
    dbNet*                   curDnet = *vIter;
    dbSet<dbSWire>           swires  = curDnet->getSWires();
    dbSet<dbSWire>::iterator sIter;
    for (sIter = swires.begin(); sIter != swires.end(); ++sIter) {
      dbSWire*                curSWire = *sIter;
      dbSet<dbSBox>           wires    = curSWire->getWires();
      dbSet<dbSBox>::iterator wIter;
      for (wIter = wires.begin(); wIter != wires.end(); ++wIter) {
        dbSBox* curWire = *wIter;
        if (curWire->isVia()) {
          dbVia* via      = curWire->getBlockVia();
          dbBox* via_bBox = via->getBBox();
          BBox   bBox
              = make_pair((via_bBox->getDX()) / 2, (via_bBox->getDY()) / 2);
          int x, y;
          curWire->getViaXY(x, y);
          dbTechLayer* via_layer = via->getBottomLayer();
          int          l         = via_layer->getRoutingLevel();
          if (1 != l) {
            m_Gmat->SetNode(x, y, l, bBox);
          }
          via_layer = via->getTopLayer();
          l         = via_layer->getRoutingLevel();
          m_Gmat->SetNode(x, y, l, bBox);
        } else {
          int                   x_loc1, x_loc2, y_loc1, y_loc2;
          dbTechLayer*          wire_layer = curWire->getTechLayer();
          int                   l          = wire_layer->getRoutingLevel();
          dbTechLayerDir::Value layer_dir  = wire_layer->getDirection();
          if (layer_dir == dbTechLayerDir::Value::HORIZONTAL) {
            y_loc1 = (curWire->yMin() + curWire->yMax()) / 2;
            y_loc2 = (curWire->yMin() + curWire->yMax()) / 2;
            x_loc1 = curWire->xMin();
            x_loc2 = curWire->xMax();
          } else {
            x_loc1 = (curWire->xMin() + curWire->xMax()) / 2;
            x_loc2 = (curWire->xMin() + curWire->xMax()) / 2;
            y_loc1 = curWire->yMin();
            y_loc2 = curWire->yMax();
          }
          if (l == 1) {  // special case for layer 1 we design a dense grid
            if (layer_dir == dbTechLayerDir::Value::HORIZONTAL) {
              int x_i;
              for (x_i = x_loc1; x_i <= x_loc2; x_i = x_i + m_node_density) {
                m_Gmat->SetNode(x_i, y_loc1, l, make_pair(0, 0));
              }
            } else {
              int y_i;
              for (y_i = y_loc1; y_i <= y_loc2; y_i = y_i + m_node_density) {
                m_Gmat->SetNode(x_loc1, y_i, l, make_pair(0, 0));
              }
            }
          } else {  // add end nodes
            m_Gmat->SetNode(x_loc1, y_loc1, l, make_pair(0, 0));
            m_Gmat->SetNode(x_loc2, y_loc2, l, make_pair(0, 0));
          }
        }
      }
    }
  }
  // insert c4 bumps as nodes
  for (int it = 0; it < m_C4Bumps.size(); ++it) {
    int   x    = get<0>(m_C4Bumps[it]);
    int   y    = get<1>(m_C4Bumps[it]);
    int   l    = get<2>(m_C4Bumps[it]);
    Node* node = m_Gmat->SetNode(x, y, l, make_pair(0, 0));
    m_C4GLoc.push_back(node->GetGLoc());
  }
  
  // All new nodes must be inserted by this point
  // initialize G Matrix
  cout << "INFO: G matrix created " << endl;
  cout << "INFO: Number of nodes: " << m_Gmat->GetNumNodes() << endl;
  m_Gmat->InitializeGmatDok();
  int warn_flag_via = 1;
  int warn_flag_layer = 1;
  for (vIter = vdd_nets.begin(); vIter != vdd_nets.end();
       ++vIter) {  // only 1 is expected?
    dbNet*                   curDnet = *vIter;
    dbSet<dbSWire>           swires  = curDnet->getSWires();
    dbSet<dbSWire>::iterator sIter;
    for (sIter = swires.begin(); sIter != swires.end();
         ++sIter) {  // only 1 is expected?
      dbSWire*                curSWire = *sIter;
      dbSet<dbSBox>           wires    = curSWire->getWires();
      dbSet<dbSBox>::iterator wIter;
      for (wIter = wires.begin(); wIter != wires.end(); ++wIter) {
        dbSBox* curWire = *wIter;
        if (curWire->isVia()) {
          dbVia* via      = curWire->getBlockVia();
          dbBox* via_bBox = via->getBBox();
          BBox   bBox
              = make_pair((via_bBox->getDX()) / 2, (via_bBox->getDY()) / 2);
          int x, y;
          curWire->getViaXY(x, y);
          dbTechLayer* via_layer = via->getBottomLayer();
          int          l         = via_layer->getRoutingLevel();
          int          l_bot     = l;

          double R = via_layer->getUpperLayer()->getResistance();
          if (R == 0.0) {
            warn_flag_via = 0;
            R = get<2>(m_layer_res[l]); /// Must figure out via resistance value
            //cout << "Via Resistance" << R << endl;
          }
          Node* node_bot = m_Gmat->GetNode(x, y, l);

          via_layer      = via->getTopLayer();
          l              = via_layer->getRoutingLevel();
          int   l_top    = l;
          Node* node_top = m_Gmat->GetNode(x, y, l);
          if (node_bot == nullptr || node_top == nullptr) {
            cout << "ERROR: null pointer received for expected node. Code may "
                    "fail ahead.\n";
            exit(1);
          } else {
            m_Gmat->SetConductance(node_bot, node_top, 1 / R);
          }
        } else {
          dbTechLayer* wire_layer = curWire->getTechLayer();
          int l  = wire_layer->getRoutingLevel();
          double       rho        = wire_layer->getResistance()
                       * double(wire_layer->getWidth())
                       / double((wire_layer->getTech())->getDbUnitsPerMicron());
          if (rho == 0.0) {
            warn_flag_layer = 0;
            rho = get<1>(m_layer_res[l]) * double(wire_layer->getWidth())
                    / double((wire_layer->getTech())->getDbUnitsPerMicron());
            //cout << "rho value " <<rho << endl;
          }
          dbTechLayerDir::Value layer_dir = wire_layer->getDirection();
          m_Gmat->GenerateStripeConductance(wire_layer->getRoutingLevel(),
                                            layer_dir,
                                            curWire->xMin(),
                                            curWire->xMax(),
                                            curWire->yMin(),
                                            curWire->yMax(),
                                            rho);
        }
      }
    }
  }
  if (warn_flag_via == 0) {
  cout << "Warning: Atleast one via resistance not found in DB. Using default value from the file "<<endl;
  }
  if (warn_flag_layer == 0) {
  cout << "Warning: Atleast one layer per unit resistance not found in DB. Using default value from the file "<<endl;
  }
}


//! Function to get the power value from OpenSTA
/*
 *\return vector of pairs of instance name 
 and its corresponding power value
*/
vector<pair<string, double>> IRSolver::GetPower()
{
  PowerInst                    power_inst;
  vector<pair<string, double>> power_report = power_inst.executePowerPerInst(
      m_sta);

  return power_report;
}
