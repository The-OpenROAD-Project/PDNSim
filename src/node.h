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
#ifndef __IRSOLVER_NODE__
#define __IRSOLVER_NODE__

#include <map>

typedef std::pair<int, int>         NodeLoc;
typedef std::pair<int, int>         BBox;
typedef int                         NodeIdx;  // TODO temp as it interfaces with SUPERLU
typedef std::pair<NodeIdx, NodeIdx> GMatLoc;
typedef struct
{
  NodeIdx                   num_rows;
  NodeIdx                   num_cols;
  std::map<GMatLoc, double> values;  // pair < col_num, row_num >
} DokMatrix;
typedef struct
{
  NodeIdx              num_rows;
  NodeIdx              num_cols;
  NodeIdx              nnz;
  std::vector<NodeIdx> row_idx;
  std::vector<NodeIdx> col_ptr;
  std::vector<double>  values;
} CscMatrix;

class Node
{
 public:
  Node() : m_loc(std::make_pair(0.0, 0.0)), m_bBox(std::make_pair(0.0, 0.0)) {}
  ~Node() {}
  int     GetLayerNum();
  void    SetLayerNum(int layer);
  NodeLoc GetLoc();
  void    SetLoc(int x, int y);
  void    SetLoc(int x, int y, int l);
  NodeIdx GetGLoc();
  void    SetGLoc(NodeIdx loc);
  void    print();
  void    SetBbox(int dX, int dY);
  BBox    GetBbox();
  // bool withinBoundingBox(NodeLoc t_nodeLoc, BBox t_bBox, int &t_dist ) {
  void   UpdateMaxBbox(int dX, int dY);
  void   setCurrent(double t_current);
  double getCurrent();
  void   addCurrentSrc(double t_current);
  void   setVoltage(double t_voltage);
  double getVoltage();

 private:
  int     m_layer;
  NodeLoc m_loc;  // layer,x,y
  NodeIdx m_node_loc{0};
  BBox    m_bBox;
  double  m_current_src{0.0};
  double  m_voltage{0.0};
};
#endif
