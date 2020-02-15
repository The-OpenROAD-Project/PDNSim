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
#include <iostream>
#include "node.h"

using namespace std;

int Node::GetLayerNum()
{
  return m_layer;
}
void Node::SetLayerNum(int t_layer)
{
  m_layer = t_layer;
}
NodeLoc Node::GetLoc()
{
  return m_loc;
}
void Node::SetLoc(int t_x, int t_y)
{
  m_loc = make_pair(t_x, t_y);
}
void Node::SetLoc(int t_x, int t_y, int t_l)
{
  SetLayerNum(t_l);
  SetLoc(t_x, t_y);
}
NodeIdx Node::GetGLoc()
{
  return m_node_loc;
}
void Node::SetGLoc(NodeIdx t_loc)
{
  m_node_loc = t_loc;
}
void Node::print()
{
  cout << "Node: " << m_node_loc << endl;
  cout << "    Location: Layer " << m_layer << ", x " << m_loc.first << ", y "
       << m_loc.second << endl;
  cout << "    Bounding box: x " << m_bBox.first << ", y " << m_bBox.second
       << endl;
  cout << "    Current: " << m_current_src << endl;
  cout << "    Voltage: " << m_voltage << endl;
}
void Node::SetBbox(int t_dX, int t_dY)
{
  m_bBox = make_pair(t_dX, t_dY);
}
BBox Node::GetBbox()
{
  return m_bBox;
}
void Node::UpdateMaxBbox(int t_dX, int t_dY)
{
  BBox nodeBbox = m_bBox;
  int  DX       = max(nodeBbox.first, t_dX);
  int  DY       = max(nodeBbox.second, t_dY);
  SetBbox(DX, DY);
}
void Node::setCurrent(double t_current)
{
  m_current_src = t_current;
}
double Node::getCurrent()
{
  return m_current_src;
}
void Node::addCurrentSrc(double t_current)
{
  double node_cur = getCurrent();
  setCurrent(node_cur + t_current);
}
void Node::setVoltage(double t_voltage)
{
  m_voltage = t_voltage;
}
double Node::getVoltage()
{
  return m_voltage;
}
