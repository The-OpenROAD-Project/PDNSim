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
#include <queue>
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

#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include "opendb/db.h"
#include "ir_solver.h"
#include "node.h"
#include "gmat.h"
#include "get_power.h"
#include "openroad/Error.hh"
#include "pdnsim/pdnsim.h"


//using namespace pdnsim;

namespace pdnsim {

using namespace std;
using ord::error;
using ord::warn;
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
using odb::dbViaParams;

//using namespace std;
using std::vector;
using Eigen::Map;
using Eigen::VectorXd;
using Eigen::SparseMatrix; 
using Eigen::SparseLU;
using Eigen::Success;

std::map<string, int> GetViaParams(dbVia* via);


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


//! Function to solve for voltage using SparseLU 
void IRSolver::SolveIR()
{
  if(!m_connection) {
    string s = "Powergrid is not connected to all instances, IR Solver may not be accurate, LVS may also fail.";
    PdnsimLogger(ERROR, 31, s);
  }
  int unit_micron = (m_db->getTech())->getDbUnitsPerMicron();
  clock_t t1, t2;
  CscMatrix*        Gmat = m_Gmat->GetGMat();
// fill A
  int     nnz     = Gmat->nnz;
  int               m    = Gmat->num_rows;
  int               n    = Gmat->num_cols;
  double* values  = &(Gmat->values[0]);
  int*    row_idx = &(Gmat->row_idx[0]);
  int*    col_ptr = &(Gmat->col_ptr[0]);
  Map<SparseMatrix<double> > A( Gmat->num_rows,
                                Gmat->num_cols,
                                Gmat->nnz,
                                col_ptr, // read-write
                                row_idx,
                                values);
    

  vector<double>    J = GetJ();
  Map<VectorXd> b(J.data(),J.size());
  VectorXd x;
  SparseLU<SparseMatrix<double> > solver;
  string s = "Starting PDN IR analysis";
  PdnsimLogger(INFO, 32, s);
  solver.compute(A);
  if(solver.info()!=Success) {
    // decomposition failed
    string s = "LU factorization of conductance matrix failed";
    PdnsimLogger(ERROR, 33, s);
    return;
  }
  x = solver.solve(b);
  if(solver.info()!=Success) {
    // solving failed
    string s = "Sparse solver fails in IR analysis";
    PdnsimLogger(ERROR, 33, s);
    return;
  }
  s = "Completed PDN IR analysis";
  PdnsimLogger(INFO, 32, s);
  ofstream ir_report;
  ir_report.open (m_out_file);
  ir_report<<"Instance name, "<<" X location, "<<" Y location, "<<" Voltage "<<"\n";
  int num_nodes         = m_Gmat->GetNumNodes();
  int node_num =0;
  double       sum_volt = 0;
  wc_voltage            = vdd;
  while(node_num < num_nodes) {
    Node* node = m_Gmat->GetNode(node_num);
    double volt = x(node_num);
    sum_volt   = sum_volt + volt;
    if (volt < wc_voltage) {
      wc_voltage = volt;
    }
    node->SetVoltage(volt);
    node_num++;
    if(node->HasInstances()) {
      NodeLoc node_loc = node->GetLoc();
      float loc_x = ((float)node_loc.first)/((float)unit_micron);
      float loc_y = ((float)node_loc.second)/((float)unit_micron);
      std::vector<dbInst*> insts = node->GetInstances();
      std::vector<dbInst*>::iterator inst_it;
      if (m_out_file != "") {
        for(inst_it = insts.begin();inst_it!=insts.end();inst_it++) {
          ir_report<<(*inst_it)->getName()<<", "<<loc_x<<", " <<loc_y<<", "<<setprecision(10)<<volt<<"\n";
        }
      }
    }
  }
  ir_report<<endl;
  ir_report.close();
  avg_voltage = sum_volt / num_nodes;
}

//! Function to add C4 bumps to the G matrix
bool IRSolver::AddC4Bump()
{
  if (m_C4Bumps.size() == 0) {
    string s = "Invalid number of voltage sources"; 
    PdnsimLogger(ERROR, 34, s);
    return false;
  }
  for (unsigned int it = 0; it < m_C4Nodes.size(); ++it) {
    NodeIdx node_loc = m_C4Nodes[it].first;
    double  voltage  = m_C4Nodes[it].second;
    m_Gmat->AddC4Bump(node_loc, it);  // add the 0th bump
    m_J.push_back(voltage);           // push back first vdd
    vdd = voltage;
  }
  return true;
}



//! Function that parses the Vsrc file
void IRSolver::ReadC4Data()
{
  int unit_micron = (m_db->getTech())->getDbUnitsPerMicron();
  string s = "Reading location of VDD and VSS sources"; 
  PdnsimLogger(INFO, 35, s);
  if(m_vsrc_file != "") {
  std::ifstream file(m_vsrc_file);
    std::string line = "";
  // Iterate through each line and split the content using delimiter
  while (getline(file, line)) {
    tuple<int, int, int, double> c4_bump;
    int                          first, second, size;
    double                       voltage;
    stringstream                 X(line);
    string                       val;
    for (int i = 0; i < 4; ++i) {
      getline(X, val, ',');
      if (i == 0) {
        first = (int) (unit_micron * stod(val));
      } else if (i == 1) {
        second = (int) (unit_micron * stod(val));
      } else if (i == 2) {
        size = (int) (unit_micron * stod(val));
      } else {
        voltage = stod(val);
      }
    }
    m_C4Bumps.push_back(make_tuple(first, second, size, voltage));
  }
  file.close();
  }
  else {
    string s = "Voltage pad location file not spcified, defaulting pad location to origin";
    PdnsimLogger(WARN, 36, s);
    m_C4Bumps.push_back(make_tuple(0,0,0,0));
  }
}



//! Function to create a J vector from the current map
bool IRSolver::CreateJ()
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
      stringstream s;
      s << "Instance " << it->first << " not found within database";
      PdnsimLogger(WARN, 36, s.str());
      continue;
    }
    int x, y;
    inst->getLocation(x, y);
    //cout << "Got location" <<endl;
    int   l      = m_bottom_layer;  // atach to the bottom most routing layer
    Node* node_J = m_Gmat->GetNode(x, y, l,true);
    NodeLoc node_loc = node_J->GetLoc();
    if( abs(node_loc.first - x) > m_node_density || abs(node_loc.second - y) > m_node_density ){
      stringstream s ;
      s <<"Instance "<< it->first<<"current node at "<<node_loc.first<<" "<<node_loc.second<<" layer "<<l<<" moved from "<<x<<" "<<y;
      PdnsimLogger(WARN, 38, s.str());
    }
    //TODO modify for ground network
    node_J->AddCurrentSrc(it->second);
    node_J->AddInstance(inst);
  }
  for (int i = 0; i < num_nodes; ++i) {
    Node* node_J = m_Gmat->GetNode(i);
    m_J[i] = -1 * (node_J->GetCurrent());  // as MNA needs negative
    // cout << m_J[i] <<endl;
  }
  string s = "Completed parsing current values";
  PdnsimLogger(INFO, 40, s);
  return true;
}

bool IRSolver::GetPowerNets() {
  dbChip*             chip  = m_db->getChip();
  dbBlock*            block = chip->getBlock();
  dbSet<dbNet>        nets  = block->getNets();
  dbSet<dbNet>::iterator nIter;
  string s = "Extracting power stripes on net "+ m_power_net;
  PdnsimLogger(INFO, 71, s);
  for (nIter = nets.begin(); nIter != nets.end(); ++nIter) {
    dbNet* curDnet = *nIter;
    dbSigType nType = curDnet->getSigType();
    if(m_power_net == "VSS") {
      if (nType == dbSigType::GROUND) {
        m_power_nets.push_back(curDnet);
      } else {
        continue;
      }
    } else if(m_power_net == "VDD") {
      if (nType == dbSigType::POWER) {
        m_power_nets.push_back(curDnet);
      } else {
        continue;
      }
    } else {
      string s = "Net not specifed as VDD or VSS";
      PdnsimLogger(ERROR, 72, s);
      return false;
    }
  }
  if(m_power_nets.size() == 0) {
    string s= "No VDD or VSS nets found";
    PdnsimLogger(CRIT, 73, s);
    return false;
  }
  return true;
}

void IRSolver::GetPowerNetWires() {
  std::vector<dbSBox*> power_wires;
  std::vector<dbNet*>::iterator vIter;
  for (vIter = m_power_nets.begin(); vIter != m_power_nets.end(); ++vIter) {
    dbNet*                   curDnet = *vIter;
    dbSet<dbSWire>           swires  = curDnet->getSWires();
    dbSet<dbSWire>::iterator sIter;
    for (sIter = swires.begin(); sIter != swires.end(); ++sIter) {
      dbSWire*                curSWire = *sIter;
      dbSet<dbSBox>           wires    = curSWire->getWires();
      dbSet<dbSBox>::iterator wIter;
      for (wIter = wires.begin(); wIter != wires.end(); ++wIter) {
        dbSBox* curWire = *wIter;
        m_power_wires.push_back(curWire);
      }
    }
  }
  if (m_power_wires.size() == 0){
    string s = "No wires found on net" + m_power_net;
    PdnsimLogger(CRIT, 72, s);
  }
}

void IRSolver::FindTopBottomPDNLayer() {
  //dbSet<dbSBox>::iterator wIter;
  int size = m_power_wires.size();
 // cout << "Vector of wires of size" << size <<end;;
  for (auto wIter = m_power_wires.begin(); wIter != m_power_wires.end(); ++wIter) {
    dbSBox* curWire = *wIter;
    int l;
    dbTechLayerDir::Value layer_dir; 
    if (curWire->isVia()) {
      dbVia* via      = curWire->getBlockVia();
      dbTechLayer* via_layer = via->getTopLayer();
      l = via_layer->getRoutingLevel();
      layer_dir = via_layer->getDirection();
    } else {
      dbTechLayer* wire_layer = curWire->getTechLayer();
      l = wire_layer->getRoutingLevel();
      layer_dir = wire_layer->getDirection();
      if (l < m_bottom_layer) {
        m_bottom_layer = l ; 
        m_bottom_layer_dir = layer_dir;
      }
    }
    if (l > m_top_layer) {
      m_top_layer = l ; 
      m_top_layer_dir = layer_dir;
    }
  }
}

void IRSolver::CreateNodes() {
  cout<<"Creating Nodes:     ";
  int progress_wires=0;
  int progress_percent=1;
  //dbSet<dbSBox>::iterator wIter;
  int num_wires = m_power_wires.size();
  for (auto wIter = m_power_wires.begin(); wIter != m_power_wires.end(); ++wIter) {
    if(progress_wires >= ((progress_percent/100.0)*num_wires)-1.0 ){
      cout<<"\b\b\b\b"<<setw(3)<<progress_percent++<<"%"<< std::flush;
    }
    progress_wires++;
    dbSBox* curWire = *wIter;
    if (curWire->isVia()) {
      CreateViaNodes(curWire);
    }
    else {
      CreateStripeNodes(curWire);
    }
  }
  cout<<endl;
  CreateRDLNodes();
  stringstream s ;
  s << "Number of nodes on net " << m_power_net <<" =" << m_Gmat->GetNumNodes();
  PdnsimLogger(INFO, 39, s.str());
}


void IRSolver::CreateRDLNodes() {
  dbTech* tech   = m_db->getTech();
  int unit_micron = tech->getDbUnitsPerMicron();
  for (unsigned int it = 0; it < m_C4Bumps.size(); ++it) {
    int x = get<0>(m_C4Bumps[it]);
    int y = get<1>(m_C4Bumps[it]);
    int size = get<2>(m_C4Bumps[it]);
    double v  = get<3>(m_C4Bumps[it]);
    std::vector<Node*> RDL_nodes;
    RDL_nodes = m_Gmat->GetRDLNodes(m_top_layer, 
                                    m_top_layer_dir,
                                    x-size/2, 
                                    x+size/2,
                                    y-size/2,
                                    y+size/2);
    if (RDL_nodes.empty() == true) {
      Node* node = m_Gmat->GetNode(x,y,m_top_layer,true);
      NodeLoc node_loc = node->GetLoc();
      double new_loc1 = ((double)node_loc.first) /((double) unit_micron);
      double new_loc2 = ((double)node_loc.second) /((double) unit_micron);
      double old_loc1 = ((double)x) /((double) unit_micron);
      double old_loc2 = ((double)y) /((double) unit_micron);
      double old_size = ((double)size) /((double) unit_micron);
      stringstream s1,s2 ;
      s1 <<"Vsrc location at x="<<std::setprecision(3)<<old_loc1<<"um , y="<<old_loc2
          <<"um and size ="<<old_size<<"um,  is not located on a power stripe.";
      PdnsimLogger(WARN, 38, s1.str());
      s2 <<"Moving to closest stripe at x="<<std::setprecision(3)<<new_loc1<<"um , y="<<new_loc2<<"um";
      PdnsimLogger(INFO, 38, s2.str());
      RDL_nodes = m_Gmat->GetRDLNodes(m_top_layer, 
                                      m_top_layer_dir,
                                      node_loc.first-size/2, 
                                      node_loc.first+size/2,
                                      node_loc.second-size/2,
                                      node_loc.second+size/2);

    }
    vector<Node*>::iterator node_it;
    for(node_it = RDL_nodes.begin(); node_it != RDL_nodes.end(); ++node_it) {
      Node* node = *node_it;
      m_C4Nodes.push_back(make_pair(node->GetGLoc(),v));
    }
  }  
}


void IRSolver::CreateStripeNodes(dbSBox* curWire) {
  int                   x_loc1, x_loc2, y_loc1, y_loc2;
  dbTechLayer*          wire_layer = curWire->getTechLayer();
  int                   l          = wire_layer->getRoutingLevel();
  dbTechLayerDir::Value layer_dir  = wire_layer->getDirection();
  if(l == m_bottom_layer){
    layer_dir = dbTechLayerDir::Value::HORIZONTAL;
  }
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
  if (l == m_bottom_layer || l == m_top_layer) {  // special case for bottom and top layers we design a dense grid
    if (layer_dir == dbTechLayerDir::Value::HORIZONTAL ) {
      int x_i;
      x_loc1 = (x_loc1/m_node_density)*m_node_density; //quantize the horizontal direction
      x_loc2 = (x_loc2/m_node_density)*m_node_density; //quantize the horizontal direction
      for (x_i = x_loc1; x_i <= x_loc2; x_i = x_i + m_node_density) {
        m_Gmat->SetNode(x_i, y_loc1, l);
      }
    } else {
      y_loc1 = (y_loc1/m_node_density)*m_node_density; //quantize the vertical direction
      y_loc2 = (y_loc2/m_node_density)*m_node_density; //quantize the vertical direction
      int y_i;
      for (y_i = y_loc1; y_i <= y_loc2; y_i = y_i + m_node_density) {
        m_Gmat->SetNode(x_loc1, y_i, l);
      }
    }
  } else {  // add end nodes
    m_Gmat->SetNode(x_loc1, y_loc1, l);
    m_Gmat->SetNode(x_loc2, y_loc2, l);
  }
}




void IRSolver::CreateViaEnclosureNode(dbTechLayer* layer, int x, int y, std::map<string, int> via_params_map, int via_layer) {
  int l = layer->getRoutingLevel();
  int x_loc1, x_loc2, y_loc1, y_loc2;
  dbTechLayerDir::Value layer_dir = layer->getDirection();
  if (m_bottom_layer != l && l != m_top_layer) {//do not set for top and bottom layers
    if (layer_dir == dbTechLayerDir::Value::HORIZONTAL) {
      y_loc1 = y;
      y_loc2 = y;
      if (via_layer == 0) {
        x_loc1 = x - (via_params_map["x_bottom_enclosure"] + via_params_map["x_cut_size"]/2);
        x_loc2 = x + (via_params_map["x_bottom_enclosure"] + via_params_map["x_cut_size"]/2);
      }
      else  {
        x_loc1 = x - (via_params_map["x_top_enclosure"] + via_params_map["x_cut_size"]/2);
        x_loc2 = x + (via_params_map["x_top_enclosure"] + via_params_map["x_cut_size"]/2);
      }
    } else {
      if (via_layer == 0) {
        y_loc1 = y - (via_params_map["y_bottom_enclosure"] + via_params_map["y_cut_size"]/2);
        y_loc2 = y + (via_params_map["y_bottom_enclosure"] + via_params_map["y_cut_size"]/2);
      }
      else {
        y_loc1 = y - (via_params_map["y_top_enclosure"] + via_params_map["y_cut_size"]/2);
        y_loc2 = y + (via_params_map["y_top_enclosure"] + via_params_map["y_cut_size"]/2);
      }
      x_loc1 = x;
      x_loc2 = x;
    }
    m_Gmat->SetNode(x_loc1, y_loc1, l);
    m_Gmat->SetNode(x_loc2, y_loc2, l);
}
}

std::map<string, int> GetViaParams(dbVia* via) {
   std::map<string, int> via_params_map;
   dbViaParams params;
   via->getViaParams(params);
   via_params_map["x_cut_size"] = params.getXCutSize();
   via_params_map["y_cut_size"] = params.getYCutSize();
   via_params_map["x_bottom_enclosure"] = params.getXBottomEnclosure();
   via_params_map["y_bottom_enclosure"] = params.getYBottomEnclosure(); 
   via_params_map["x_top_enclosure"] = params.getXTopEnclosure();
   via_params_map["y_top_enclosure"] = params.getYTopEnclosure();
   via_params_map["num_via_rows"] = params.getNumCutRows();
   via_params_map["num_via_cols"] = params.getNumCutCols();
   return via_params_map;

}

void IRSolver::CreateViaNodes(dbSBox* curWire) {
  dbVia* via      = curWire->getBlockVia();
  std::map<string, int> via_params_map;
  int check_params = via->hasParams();
  if(check_params == 1) {
    via_params_map = GetViaParams(via);
  }
  else {
    stringstream s;
    s<<"Via " << via->getName() << "does not have parameters defined"; 
    PdnsimLogger(ERROR, 53, s.str());
  }
  int x, y;
  curWire->getViaXY(x, y);
  // Create nodes for top layer of via
  dbTechLayer* via_layer = via->getBottomLayer();
  CreateViaEnclosureNode(via_layer, x, y, via_params_map, 0);
  m_Gmat->SetNode(x, y, via_layer->getRoutingLevel());
  // Create nodes for bottom layer of via
  via_layer = via->getTopLayer();
  CreateViaEnclosureNode(via_layer, x, y, via_params_map, 1);
  m_Gmat->SetNode(x, y, via_layer->getRoutingLevel());

}


bool IRSolver::CreateConnections() {
  cout << "Creating Connections:     ";
  m_Gmat->InitializeGmatDok(m_C4Nodes.size());
  int progress_wires=0;
  int progress_percent=1;
  int err_flag_layer = 1;
  //dbSet<dbSBox>::iterator wIter;
  int num_wires = m_power_wires.size();
  for (auto wIter = m_power_wires.begin(); wIter != m_power_wires.end(); ++wIter) {
    if(progress_wires >= ((progress_percent/100.0)*num_wires)-1.0 ){
      cout<<"\b\b\b\b"<<setw(3)<<progress_percent++<<"%"<< std::flush;
    }
    progress_wires++;
    dbSBox* curWire = *wIter;
    if (curWire->isVia()) {
      if (!CreateViaConnections(curWire))
        return false;
    } else {
    if(!CreateStripeConnections(curWire))
      return false;
    }
 }
 cout<<endl;
 return true;
}

Node* IRSolver::GetViaNode(dbTechLayer* via_layer, int x, int y) {
  int l = via_layer->getRoutingLevel();
  bool top_or_bottom = ((l == m_bottom_layer) || (l == m_top_layer));
  Node* node = m_Gmat->GetNode(x, y, l,top_or_bottom);
  NodeLoc node_loc = node->GetLoc();
  if( abs(node_loc.first - x) > m_node_density || abs(node_loc.second - y) > m_node_density ){
    stringstream s;
    s<<"Node at "<<node_loc.first<<" "<<node_loc.second<<" layer "<<l<<" moved from "<<x<<" "<<y;
    PdnsimLogger(WARN, 38, s.str());
  }
  return node;
}

bool IRSolver::CreateStripeConnections(dbSBox* curWire) {
  dbTechLayer* wire_layer = curWire->getTechLayer();
  int l  = wire_layer->getRoutingLevel();
  double rho = GetLayerRho(wire_layer);
   if (rho == 0 && !m_connection_only) {
    stringstream s;
    s<< "Resistance of via layer " << wire_layer->getName() << "not set in DB";
    PdnsimLogger(ERROR, 51, s.str());
    return false;
   } 
  dbTechLayerDir::Value layer_dir = wire_layer->getDirection();
  if (l == m_bottom_layer){//ensure that the bootom layer(rail) is horizontal
    layer_dir = dbTechLayerDir::Value::HORIZONTAL;
  }
  int x_loc1 = curWire->xMin();
  int x_loc2 = curWire->xMax();
  int y_loc1 = curWire->yMin();
  int y_loc2 = curWire->yMax();
  if (l == m_bottom_layer || l == m_top_layer) {  // special case for bottom and top layers we design a dense grid
    if (layer_dir == dbTechLayerDir::Value::HORIZONTAL ) {
      x_loc1 = (x_loc1/m_node_density)*m_node_density; //quantize the horizontal direction
      x_loc2 = (x_loc2/m_node_density)*m_node_density; //quantize the horizontal direction
    } else {
      y_loc1 = (y_loc1/m_node_density)*m_node_density; //quantize the vertical direction
      y_loc2 = (y_loc2/m_node_density)*m_node_density; //quantize the vertical direction
    }
  }
  m_Gmat->GenerateStripeConductance(l, layer_dir, x_loc1, x_loc2, y_loc1, y_loc2, rho);
  return true;
}

bool IRSolver::CreateViaConnections(dbSBox* curWire) {
  dbVia* via      = curWire->getBlockVia();
  std::map<string, int> via_params_map;
  int check_params = via->hasParams();
  if(check_params == 1) {
    via_params_map = GetViaParams(via);
  }
  int x, y;
  curWire->getViaXY(x, y);
  dbTechLayer* via_bot_layer = via->getBottomLayer();
  dbTechLayer* via_top_layer = via->getTopLayer();
  Node* node_bot;
  Node* node_top;
  node_bot = GetViaNode(via_bot_layer, x, y);
  node_top = GetViaNode(via_top_layer, x, y);
  double R = via_bot_layer->getUpperLayer()->getResistance();
  R = R/(via_params_map["num_via_rows"] * via_params_map["num_via_cols"]);
  if (node_bot == nullptr || node_top == nullptr) {
    stringstream s;
    s<< "Null pointer received for an expected node at location: "<< x << "," << y;
    PdnsimLogger(ERROR, 52, s.str());
    return false;
  } 
  if(R <= 1e-15){ //if the resitance was not set.
    m_Gmat->SetConductance(node_bot, node_top, 0);
    if (!m_connection_only) {
      stringstream s;
      s<< "Resistance of via layer " << via_bot_layer->getName() << "not set in DB";
      PdnsimLogger(ERROR, 51, s.str());
      return false;
    }
  } else {
    m_Gmat->SetConductance(node_bot, node_top, 1 / R);
  }
  bool res1 = CreateViaEnclosureConnections(via_bot_layer, x, y, via_params_map, 0);
  bool res2 = CreateViaEnclosureConnections(via_top_layer, x, y, via_params_map, 1);
  return (res1 && res2);
}


double IRSolver::GetLayerRho(dbTechLayer* via_layer) {
  int unit_micron = (m_db->getTech())->getDbUnitsPerMicron();
  double rho  = via_layer->getResistance() * double(via_layer->getWidth()) / double(unit_micron);
  if (rho <= 1e-15) {
    rho = 0;
  }
  return rho;
}

bool IRSolver::CreateViaEnclosureConnections(dbTechLayer* via_layer, int x, int y,std::map<std::string, int> via_params_map, int via_layer_num) {
   dbTechLayerDir::Value layer_dir  = via_layer->getDirection();
   int l = via_layer->getRoutingLevel();
   double rho = GetLayerRho(via_layer);
   if (rho == 0 && !m_connection_only) {
    stringstream s;
    s<< "Resistance of via layer " << via_layer->getName() << "not set in DB" ;
    PdnsimLogger(ERROR, 51, s.str());
    return false;
   } 
   if(l != m_bottom_layer || l != m_top_layer) {
     int x_loc1,x_loc2,y_loc1,y_loc2;
     if (layer_dir == dbTechLayerDir::Value::HORIZONTAL) {
       y_loc1 = y - via_params_map["y_cut_size"]/2;
       y_loc2 = y + via_params_map["y_cut_size"]/2;
       if(via_layer_num ==0) {
       x_loc1 = x - (via_params_map["x_bottom_enclosure"] + via_params_map["x_cut_size"]/2);
       x_loc2 = x + (via_params_map["x_bottom_enclosure"] + via_params_map["x_cut_size"]/2);
       }
       else {
       x_loc1 = x - (via_params_map["x_top_enclosure"] + via_params_map["x_cut_size"]/2);
       x_loc2 = x + (via_params_map["x_top_enclosure"] + via_params_map["x_cut_size"]/2);
       }
     } else {
       if(via_layer_num ==0) {
       y_loc1 = y - (via_params_map["y_bottom_enclosure"] + via_params_map["y_cut_size"]/2);
       y_loc2 = y + (via_params_map["y_bottom_enclosure"] + via_params_map["y_cut_size"]/2);
       } 
       else {
       y_loc1 = y - (via_params_map["y_top_enclosure"] + via_params_map["y_cut_size"]/2);
       y_loc2 = y + (via_params_map["y_top_enclosure"] + via_params_map["y_cut_size"]/2);
       }
       x_loc1 = x - via_params_map["x_cut_size"]/2;
       x_loc2 = x + via_params_map["x_cut_size"]/2;
     }
     m_Gmat->GenerateStripeConductance(via_layer->getRoutingLevel(), layer_dir, x_loc1, x_loc2, y_loc1, y_loc2, rho);
   }
   return true;
}

//! Function to create a G matrix using the nodes
bool IRSolver::CreateGmat() {
  dbTech*                      tech   = m_db->getTech();
  int num_routing_layers = tech->getRoutingLayerCount();
  bool check_connections;
  m_Gmat   = new GMat(num_routing_layers);
  if(!GetPowerNets()) {
    string s = "Power nets not found in design";
    PdnsimLogger(ERROR, 50, s);
    return false;
  }
  else {
    GetPowerNetWires();
    FindTopBottomPDNLayer();
  }
  CreateNodes();
  check_connections = CreateConnections();
  if (!check_connections && !m_connection_only) {
    string s = "Atleast one via layer or metal layer resistance not found in DB. Check the LEF or set it with a odb::setResistance command";
    PdnsimLogger(ERROR, 51, s);
    return false;
  }
  return true;
}




bool IRSolver::CheckConnectivity()
{
  std::vector<std::pair<NodeIdx,double>>::iterator c4_node_it;
  int x,y;
  CscMatrix*        Amat = m_Gmat->GetAMat();
  int num_nodes = m_Gmat->GetNumNodes();

  dbTech* tech   = m_db->getTech();
  int unit_micron = tech->getDbUnitsPerMicron();

  for(c4_node_it = m_C4Nodes.begin(); c4_node_it != m_C4Nodes.end() ; c4_node_it++){
    Node* c4_node = m_Gmat->GetNode((*c4_node_it).first);
    std::queue<Node*> node_q;
    node_q.push(c4_node);
    while(!node_q.empty()) {
      NodeIdx col_loc, n_col_loc;
      Node* node = node_q.front();
      node_q.pop();
      node->SetConnected();
      NodeIdx col_num = node->GetGLoc();
      col_loc  = Amat->col_ptr[col_num];
      if(col_num < Amat->col_ptr.size()-1) {
        n_col_loc  = Amat->col_ptr[col_num+1];
      } else {
        n_col_loc  = Amat->row_idx.size() ;
      }
      std::vector<NodeIdx> col_vec(Amat->row_idx.begin()+col_loc,
                                   Amat->row_idx.begin()+n_col_loc);


      std::vector<NodeIdx>::iterator col_vec_it;
      for(col_vec_it = col_vec.begin(); col_vec_it != col_vec.end(); col_vec_it++){
        if(*col_vec_it<num_nodes) {
          Node* node_next = m_Gmat->GetNode(*col_vec_it);
          if(!(node_next->GetConnected())) {
            node_q.push(node_next);
          }
        }
      }
    }
  }
  int uncon_err_cnt = 0;
  int uncon_err_flag = 0;
  int uncon_inst_cnt = 0;
  int uncon_inst_flag = 0;
  std::vector<Node*> node_list = m_Gmat->GetAllNodes();
  std::vector<Node*>::iterator node_list_it;
  bool unconnected_node =false;
  for(node_list_it = node_list.begin(); node_list_it != node_list.end(); node_list_it++){
    if(!(*node_list_it)->GetConnected()){
      uncon_err_cnt++;
      NodeLoc node_loc = (*node_list_it)->GetLoc();
      float loc_x = ((float)node_loc.first)/((float)unit_micron);
      float loc_y = ((float)node_loc.second)/((float)unit_micron);

      //if(uncon_err_cnt>25 && uncon_err_flag ==0 ) {
      //  uncon_err_flag =1;
      //  cout<<"Error display limit reached, suppressing further unconnected node error messages"<<endl;
      //} else if( uncon_err_flag ==0) {
        //cout<<"node_not_connected ================================="<<endl;
        unconnected_node =true;
        stringstream s ;
        s <<"Unconnected PDN node on net " << m_power_net<<" at location x:"<<loc_x<<"um, y:"
            <<loc_y<<"um ,layer: "<<(*node_list_it)->GetLayerNum();
        PdnsimLogger(ERROR, 74, s.str());
      //}
      //if(uncon_inst_cnt>25 && uncon_inst_flag ==0 ) {
      //  uncon_inst_flag =1;
      //  cout<<"Error display limit reached, suppressing further unconnected instance error messages"<<endl;
      //} else if( uncon_inst_flag ==0) {
        if((*node_list_it)->HasInstances()){
          std::vector<dbInst*> insts = (*node_list_it)->GetInstances();
          std::vector<dbInst*>::iterator inst_it;
          for(inst_it = insts.begin();inst_it!=insts.end();inst_it++) {
            uncon_inst_cnt++;
            stringstream s ;
            s<<"ERROR: Instance: "<< (*inst_it)->getName() <<"at location x:"<<loc_x<<"um, y:"
              <<loc_y<<"um ,layer: "<<(*node_list_it)->GetLayerNum();
            PdnsimLogger(ERROR, 74, s.str());
          }
        }
      //}
    }
  }
  if(unconnected_node == false){
    string s;
    s = "No dangling stripe found on net " + m_power_net;
    PdnsimLogger(INFO, 75, s);
    s = "Connection between all PDN nodes established on net " + m_power_net;
    PdnsimLogger(INFO, 75, s);
  }
  return !unconnected_node;
}

int IRSolver::GetConnectionTest(){
  if(m_connection){
    return 1;
  } else {
    return 0;
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

bool IRSolver::GetResult(){
  return m_result; 
}

int IRSolver::PrintSpice() {
  DokMatrix*        Gmat = m_Gmat->GetGMatDOK();
  map<GMatLoc, double>::iterator it;
  
  ofstream pdnsim_spice_file;
  pdnsim_spice_file.open (m_spice_out_file);
  if (!pdnsim_spice_file.is_open()) {
    string s = "File did not open";
    PdnsimLogger(ERROR, 41, s);
    return 0;
  }
  vector<double>    J = GetJ();
  int num_nodes = m_Gmat->GetNumNodes();
  int resistance_number = 0;
  int voltage_number = 0;
  int current_number = 0; 

  NodeLoc node_loc;
  for(it = Gmat->values.begin(); it!= Gmat->values.end(); it++){
    NodeIdx col = (it->first).first;
    NodeIdx row = (it->first).second;
    if(col <= row) {
      continue; //ignore lower half and diagonal as matrix is symmetric
    }
    double cond = it->second;           // get cond value
    if(abs(cond) < 1e-15){            //ignore if an empty cell
      continue;
    }

    string net_name = "vdd";
    if(col < num_nodes) { //resistances
      double resistance = -1/cond;

      Node* node1 = m_Gmat->GetNode(col); 
      Node* node2 = m_Gmat->GetNode(row); 
      node_loc = node1->GetLoc();
      int x1 = node_loc.first;
      int y1 = node_loc.second;
      int l1 = node1->GetLayerNum();
      string node1_name = net_name + "_" + to_string(x1) + "_" + to_string(y1) + "_" + to_string(l1);

      node_loc = node2->GetLoc();
      int x2 = node_loc.first;
      int y2 = node_loc.second;
      int l2 = node2->GetLayerNum();
      string node2_name = net_name + "_" + to_string(x2) + "_" + to_string(y2) + "_" + to_string(l2);
      
      string resistance_name = "R" + to_string(resistance_number); 
      resistance_number++;

      pdnsim_spice_file<< resistance_name <<" "<< node1_name << " " << node2_name <<" "<< to_string(resistance) <<endl;

      double current = node1->GetCurrent();
      string current_name = "I" + to_string(current_number); 
      if(abs(current)> 1e-18) {
        pdnsim_spice_file<< current_name <<" "<< node1_name << " " << 0 <<" "<< current <<endl;
        current_number++;
      }


    } else { //voltage
      Node* node1 = m_Gmat->GetNode(row); //VDD location 
      node_loc = node1->GetLoc();
      double voltage = J[col];
      int x1 = node_loc.first;
      int y1 = node_loc.second;
      int l1 = node1->GetLayerNum();
      string node1_name = net_name + "_" + to_string(x1) + "_" + to_string(y1) + "_" + to_string(l1);
      string voltage_name = "V" + to_string(voltage_number); 
      voltage_number++;
      pdnsim_spice_file<< voltage_name <<" "<< node1_name << " 0 " << to_string(voltage) <<endl;
    }
  } 
  
  pdnsim_spice_file<<".OPTION NUMDGT=6"<<endl;
  pdnsim_spice_file<<".OP"<<endl;
  pdnsim_spice_file<<".END"<<endl;
  pdnsim_spice_file<<endl;
  pdnsim_spice_file.close();
  return 1;
}

bool IRSolver::Build() {
  bool res = true;
  m_connection_only = false;
  ReadC4Data();
  if(res) {
    res = CreateGmat(); 
  }
  if(res) {
    res = CreateJ();
  }
  if(res) {
    res = AddC4Bump();
  }
  if(res) {
    res = m_Gmat->GenerateCSCMatrix();
    res = m_Gmat->GenerateACSCMatrix();
  }
  if(res) {
    m_connection = CheckConnectivity();
    res = m_connection;
  }
  m_result = res;
  return m_result;
}

bool IRSolver::BuildConnection() {
  bool res = true;
  m_connection_only = true;
  ReadC4Data();
  if(res) {
    res = CreateGmat(); 
  }
  if(res) {
    res = AddC4Bump();
  }
  if(res) {
    res = m_Gmat->GenerateACSCMatrix();
  }
  if(res) {
    m_connection = CheckConnectivity();
    res = m_connection;
  }
  m_result = res;
  return m_result;
}
}
