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
