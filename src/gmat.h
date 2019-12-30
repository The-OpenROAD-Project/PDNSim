#ifndef __IRSOLVER_GMAT_
#define __IRSOLVER_GMAT_
#include "node.h"
#include "db.h"

typedef std::map<int, std::map<int,Node*>> NodeMap;

class GMat {
public:
    GMat(int t_num_layers,int t_numC4)
        :m_num_layers(t_num_layers),m_layer_maps(t_num_layers+1,NodeMap()),m_numC4(t_numC4) { //as it start from 0 and everywhere we use layer
    }
    ~GMat() {
        while(!m_G_mat_nodes.empty())
        {
          delete m_G_mat_nodes.back();
          m_G_mat_nodes.pop_back();
        }
    }
    Node* GetNode(NodeIdx t_node);
    Node* GetNode(int t_x, int t_y, int t_l);
    void SetNode(NodeIdx t_node_loc, Node* t_node);
    Node* SetNode(int t_x, int t_y, int t_layer, BBox t_bBox);
    void InsertNode(Node* t_node);
    void print();
    void SetConductance(Node* t_node1, Node* t_node2, double t_cond);
    void InitializeGmatDok();
    NodeIdx GetNumNodes();
    CscMatrix* GetGMat();
    void GenerateStripeConductance(int t_l, odb::dbTechLayerDir::Value layer_dir, int t_x_min, int t_x_max, int t_y_min, int t_y_max,double t_rho);
    void AddC4Bump(int t_loc,int t_C4Num);
    void GenerateCSCMatrix();
private:
    NodeIdx m_n_nodes{0};
    int m_num_layers;
    DokMatrix m_G_mat_dok;
    CscMatrix m_G_mat_csc;
    std::vector<Node*> m_G_mat_nodes;
    std::vector<NodeMap> m_layer_maps;
    int m_numC4{1}; 
    double GetConductance(NodeIdx t_row, NodeIdx t_col);
    void m_setConductance(NodeIdx t_row, NodeIdx t_col, double t_cond); 
    Node* m_nearestYNode(NodeMap::iterator x_itr,int t_y);
    double m_getConductivity(double width,double length,double rho); 

};


#endif
