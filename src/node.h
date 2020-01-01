#ifndef __IRSOLVER_NODE__
#define __IRSOLVER_NODE__ 

#include <map>

typedef std::pair<int,int> NodeLoc;
typedef std::pair<int,int> BBox;
typedef int NodeIdx; //TODO temp as it interfaces with SUPERLU
typedef std::pair<NodeIdx, NodeIdx> GMatLoc;
typedef struct {
    NodeIdx num_rows;
    NodeIdx num_cols;
    std::map<GMatLoc,double> values; // pair < col_num, row_num >
} DokMatrix;
typedef struct {
    NodeIdx num_rows;
    NodeIdx num_cols;
    NodeIdx nnz; 
    std::vector<NodeIdx> row_idx;
    std::vector<NodeIdx> col_ptr;
    std::vector<double> values;
} CscMatrix;

class Node {
public:
    Node()
        :m_loc(std::make_pair(0.0,0.0)), m_bBox(std::make_pair(0.0,0.0)){ 
        }  
    ~Node() { 
    }
    int GetLayerNum();
    void SetLayerNum(int layer);
    NodeLoc GetLoc();
    void SetLoc(int x, int y);
    void SetLoc(int x, int y, int l);
    NodeIdx GetGLoc();
    void SetGLoc(NodeIdx loc);
    void print();
    void SetBbox(int dX, int dY);
    BBox GetBbox();
    //bool withinBoundingBox(NodeLoc t_nodeLoc, BBox t_bBox, int &t_dist ) {
    void UpdateMaxBbox(int dX, int dY);
    void setCurrent(double t_current);
    double getCurrent();
    void addCurrentSrc(double t_current);
    void setVoltage(double t_voltage);
    double getVoltage();

private:
    int m_layer ;
    NodeLoc m_loc; //layer,x,y
    NodeIdx m_node_loc{0} ;
    BBox m_bBox;
    double m_current_src{0.0};
    double m_voltage{0.0};
};
#endif
