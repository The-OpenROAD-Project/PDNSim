#include <vector>
#include "db.h"
#include <math.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <map>
#include <fstream>
#include <iomanip>

#include "slu_ddefs.h"


using odb::dbDatabase;
using odb::dbSet;
using odb::dbTech;
using odb::dbChip;
using odb::dbBlock;
using odb::dbNet;
using odb::dbSigType;
using odb::dbVia;
using odb::dbSBox;
using odb::dbBox;
using odb::dbTechLayer;
using odb::dbTechLayerDir;
using odb::dbSWire;

using namespace std;
using std::vector;

namespace irsolver {
typedef pair<int,int> BBox;
typedef pair<int,int> NodeLoc;
typedef int NodeIdx; //TODO temp as it interfaces with SUPERLU
//typedef vector<NodeLoc,double> C4Bump;
typedef pair<NodeIdx,NodeIdx> GMatLoc;
typedef struct {
    NodeIdx num_rows;
    NodeIdx num_cols;
    map<GMatLoc,double> values; // pair < col_num, row_num >
} DokMatrix;
typedef struct {
    NodeIdx num_rows;
    NodeIdx num_cols;
    NodeIdx nnz; 
    vector<NodeIdx> row_idx;
    vector<NodeIdx> col_ptr;
    vector<double> values;
} CscMatrix;

class Node {
public:
    Node()
        :m_loc(make_pair(0.0,0.0)), m_bBox(make_pair(0.0,0.0))
    {   
        
    }
    ~Node() {
    }
    int getLayerNum() {
        return m_layer;
    }
    void setLayerNum(int t_layer) {
        m_layer = t_layer;
    }
    NodeLoc getLoc() {
        return m_loc;
    }
    void setLoc(int t_x,int t_y) {
        m_loc = make_pair(t_x,t_y);
    }
    void setLoc(int t_x,int t_y,int t_l) {
        setLayerNum(t_l);
        setLoc(t_x,t_y);
    }
    NodeIdx getGLoc() {
        return m_node_loc;
    }
    bool setGLoc(NodeIdx t_loc) {
        m_node_loc =t_loc;
        //TODO should we insert checks? or make it void
        return true;
    }
    void print(){
        cout <<"Node: "<<m_node_loc<<endl;
        cout <<"    location: Layer "<<m_layer<<", x "<<m_loc.first<<", y "<<m_loc.second<<endl; 
        cout <<"    Bounding box: x "<<m_bBox.first<<", y "<<m_bBox.second<<endl; 
    }
    void setBbox(int t_dX, int t_dY){
         m_bBox = make_pair(t_dX,t_dY);
    }
    BBox getBbox(){
         return m_bBox;
    }
    bool withinBoundingBox(NodeLoc t_nodeLoc, BBox t_bBox, int &t_dist ) {
        NodeLoc nodeLoc = m_loc;
        BBox nodeBbox = m_bBox;
        //cout<<"vals x "<<t_nodeLoc.first<<" "<< nodeLoc.first<<endl;
        //cout<<"vals y "<<t_nodeLoc.second<<" "<< nodeLoc.second<<endl;
        int DX = abs(t_nodeLoc.first - nodeLoc.first);
        int DY = abs(t_nodeLoc.second - nodeLoc.second);
        //cout<<"distances DX "<<DX<<" DY"<<DY<<" box1 "<<t_bBox.first <<" "<<t_bBox.second  <<" box 2"<<nodeBbox.first <<" "<< nodeBbox.second<<endl;
        bool cond =( (DX<=t_bBox.first || DX <= nodeBbox.first) 
               &&(DY<=t_bBox.second || DY <= nodeBbox.second) );
        if(cond) {
            t_dist = DX+DY;
        }
            return cond;
        
    }
    void updateMaxBbox(int t_dX,int t_dY) {
        BBox nodeBbox = m_bBox;
        int DX = max(nodeBbox.first ,t_dX);
        int DY = max(nodeBbox.second,t_dY);
        setBbox(DX,DY);
    }

private:
    int m_layer ;
    NodeLoc m_loc; //layer,x,y
    NodeIdx m_node_loc{0} ;
    BBox m_bBox;
    double m_current_src;
    double m_voltage;
};

typedef map<int,map<int,Node*>> NodeMap;

class GMat
{
public:
    GMat(int t_num_layers)
        :m_num_layers(t_num_layers),m_layer_maps(t_num_layers+1,NodeMap()) //as it start from 0 and everywhere we use layer.
    {
    }
    ~GMat()
    {
        while(!m_G_mat_nodes.empty())
        {
          delete m_G_mat_nodes.back();
          m_G_mat_nodes.pop_back();
        }
    }
    Node* getNode(NodeIdx t_node) {
        if( 0 <= t_node && m_n_nodes > t_node) {
            return m_G_mat_nodes[t_node];
        } else {
            return nullptr;
        }
    }
    Node* getNode(int t_x, int t_y, int t_l) {
        NodeMap &layer_map = m_layer_maps[t_l];
        if(t_l !=1) {
            NodeMap::iterator x_itr = layer_map.find(t_x);
            if(x_itr!=layer_map.end()) {
                map<int,Node*>::iterator y_itr = x_itr->second.find(t_y);
                if(y_itr != x_itr->second.end()){
                    //cout<<"found\n";
                    return y_itr->second;
                } else {
                    cout<<"not found y \n";
                    return nullptr;
                }
            } else  {
                cout<<"not found x\n";
                return nullptr;
            } 
        } else {
            NodeMap::iterator x_itr = layer_map.lower_bound(t_x);
            vector<pair<int,Node*>> node_dist_vector;
            if(layer_map.size() == 1 || x_itr==layer_map.end() || x_itr==layer_map.begin() ) {
                if(layer_map.size() == 1 ){
                    x_itr = layer_map.begin();
                } else if (x_itr==layer_map.end()) {
                    x_itr = prev(x_itr);
                } else { //do nothing as x_itr has the correct value
                }
                return m_nearestYNode(x_itr,t_y);

            } else {
                NodeMap::iterator x_prev;
                x_prev = prev(x_itr);
                Node* node1 = m_nearestYNode(x_itr,t_y);
                Node* node2 = m_nearestYNode(x_prev,t_y);
                NodeLoc node1_loc = node1->getLoc();
                NodeLoc node2_loc = node2->getLoc();
                int dist1 = abs(node1_loc.first - t_x) + abs(node1_loc.second -t_y);
                int dist2 = abs(node2_loc.first - t_x) + abs(node2_loc.second -t_y);
                if(dist1 < dist2) {
                   return node1; 
                } else {
                   return node2; 
                }
            }
        }
    }
    void setNode(NodeIdx t_node_loc, irsolver::Node* t_node) {
        if( 0 <= t_node_loc && m_n_nodes > t_node_loc) {
            t_node->setGLoc(t_node_loc);
            m_G_mat_nodes[t_node_loc] = t_node;
        }
    }
    void insertNode(irsolver::Node* t_node) {
        t_node->setGLoc(m_n_nodes);
        int layer = t_node->getLayerNum();
        NodeLoc nodeLoc = t_node->getLoc();
        NodeMap& layer_map = m_layer_maps[layer];
        layer_map[nodeLoc.first][nodeLoc.second] = t_node;
        m_G_mat_nodes.push_back(t_node);
        m_n_nodes++;
    }

    Node* setNode(int t_x, int t_y, int t_layer, BBox t_bBox) {
        NodeMap& layer_map = m_layer_maps[t_layer];
        //cout<<"set node params l,x,y bbox"<<t_x<<" "<<t_y<<" "<<t_layer<<" "<<t_bBox.first<<" "<<t_bBox.second<< " \n";
        if(layer_map.empty()){
            
            Node* node = new Node();
            node->setLoc(t_x,t_y,t_layer);
            node->updateMaxBbox(t_bBox.first,t_bBox.second);
            insertNode(node);
            //cout<<"layer empty inserting node\n";
            //node->print();
            return(node);
        }
        NodeMap::iterator x_itr = layer_map.find(t_x);
        if(x_itr!=layer_map.end()) {
            map<int,Node*>::iterator y_itr = x_itr->second.find(t_y);
            if(y_itr != x_itr->second.end()){
                Node* node = y_itr->second;
                node->updateMaxBbox(t_bBox.first,t_bBox.second);
                //cout<<"Node exists\n";
                return(node);
            }else{
                Node* node = new Node();
                node->setLoc(t_x,t_y,t_layer);
                node->updateMaxBbox(t_bBox.first,t_bBox.second);
                insertNode(node);
                //cout<<"Creating Node new y\n";
                return(node);
            }
            
        } else {
            Node* node = new Node();
            node->setLoc(t_x,t_y,t_layer);
            node->updateMaxBbox(t_bBox.first,t_bBox.second);
            insertNode(node);
            //cout<<"Creating Node new x\n";
            return(node);
        }
    }

    void print(){
        std::cout<<"GMat obj, with "<<m_n_nodes<<" nodes\n";
        for(NodeIdx i =0; i<m_n_nodes; i++) {
            Node* node_ptr = m_G_mat_nodes[i];
            if( node_ptr != nullptr) {
                node_ptr->print();
            }
        }
    }
    void setConductance(irsolver::Node* t_node1, irsolver::Node* t_node2, double t_cond) {
        //cout<<"1";
        //fflush(stdout);
        NodeIdx node1_r = t_node1->getGLoc();
        NodeIdx node2_r = t_node2->getGLoc();
        //cout<<"2 loc1 "<<node1_r<<" loc2 "<<node2_r<<" ";
        //fflush(stdout);
        double node11_cond = m_getConductance(node1_r,node1_r);
        double node22_cond = m_getConductance(node2_r,node2_r);
        double node12_cond = m_getConductance(node1_r,node2_r);
        double node21_cond = m_getConductance(node2_r,node1_r);
        //cout<<"3";
        //fflush(stdout);
        m_setConductance(node1_r,node1_r,node11_cond+t_cond);
        m_setConductance(node2_r,node2_r,node22_cond+t_cond);
        m_setConductance(node1_r,node2_r,node12_cond-t_cond);
        m_setConductance(node2_r,node1_r,node21_cond-t_cond);
        //cout<<"4";
        //fflush(stdout);
    }
    void initializeGmatDok(){
        if(m_n_nodes<=0) {
           cout<<"ERROR: no nodes in object initialization stopped.\n"; 
           exit(1);
        }else{
            m_G_mat_dok.num_cols = m_n_nodes+m_numC4;
            m_G_mat_dok.num_rows = m_n_nodes+m_numC4;
        }
    }
    NodeIdx getNumNodes() {//debug
        return m_n_nodes;
    }
    CscMatrix* getGMat() {//Nodes debug
        return &m_G_mat_csc;
    }
    DokMatrix* getGMatDok() {//Nodes debug
        return &m_G_mat_dok;
    }

    void generateStripeConductance(int t_l,dbTechLayerDir::Value layer_dir, int t_x_min, int t_x_max, int t_y_min, int t_y_max,double t_rho){
        NodeMap &layer_map = m_layer_maps[t_l];
        //cout<<"RHO "<<t_rho<<endl;
        if(layer_dir == dbTechLayerDir::Value::HORIZONTAL){
            NodeMap::iterator x_itr;
            NodeMap::iterator x_prev;
            int y_loc = (t_y_min+t_y_max)/2;
            int i = 0;
            for( x_itr=layer_map.lower_bound(t_x_min); x_itr->first <= t_x_max && x_itr!=layer_map.end(); ++x_itr){
                if(i == 0 ){
                    i=1;
                } else {
                    Node* node1 = (x_itr->second).at(y_loc);                    
                    Node* node2 = (x_prev->second).at(y_loc);
                    int width = t_y_max-t_y_min;
                    int length = x_itr->first-x_prev->first;
                    double cond = m_getConductivity(width,length,t_rho);
                    //cout<<"layer hor "<<t_l<<" cond "<<cond<<endl;
                    setConductance(node1,node2,cond);
                }
                x_prev = x_itr;
            }            
        } else {
            int x_loc = (t_x_min+t_x_max)/2;
            map<int,Node*> y_map = layer_map.at(x_loc);
            map<int,Node*>::iterator y_itr;
            map<int,Node*>::iterator y_prev;
            int i = 0;
            for( y_itr=y_map.lower_bound(t_y_min); y_itr->first <= t_y_max && y_itr!=y_map.end(); ++y_itr){
                if(i == 0 ){
                    i=1;
                } else {
                    Node* node1 = y_itr->second;                    
                    Node* node2 = y_prev->second;                    
                    int width = t_x_max-t_x_min;
                    int length = y_itr->first-y_prev->first;
                    double cond = m_getConductivity(width,length,t_rho);
                    //cout<<"layer ver "<<t_l<<" cond "<<cond<<endl;
                    setConductance(node1,node2,cond);
                }
                y_prev = y_itr;
            }
        }
        
    }
    void addC4Bump(int t_loc,int t_C4Num){
        m_setConductance(t_loc, t_C4Num+m_n_nodes,1);     
        m_setConductance(t_C4Num+m_n_nodes,t_loc,1);     
    }
    void setNumC4Bumps(int t_numC4){
        m_numC4 = t_numC4;
    }
    int getNumC4Bumps(){
        return m_numC4;
    }
    void generateCSCMatrix(){
        m_G_mat_csc.num_cols = m_G_mat_dok.num_cols; 
        m_G_mat_csc.num_rows = m_G_mat_dok.num_rows; 
        m_G_mat_csc.nnz = 0;

        for(NodeIdx col = 0; col < m_G_mat_csc.num_cols; ++col){
            m_G_mat_csc.col_ptr.push_back(m_G_mat_csc.nnz);
            map<GMatLoc,double>::iterator it;
            map<GMatLoc,double>::iterator it_lower = m_G_mat_dok.values.lower_bound(make_pair(col,0));    
            map<GMatLoc,double>::iterator it_upper = m_G_mat_dok.values.upper_bound(make_pair(col,m_G_mat_csc.num_rows));    
            for( it = it_lower; it != it_upper; ++it ) {
                m_G_mat_csc.values.push_back( it->second ); //push back value
                m_G_mat_csc.row_idx.push_back( (it->first).second); // push back row idx
                m_G_mat_csc.nnz++;
            }
        }
    }
    
private:
    NodeIdx m_n_nodes{0};
    int m_num_layers;
    DokMatrix m_G_mat_dok;
    CscMatrix m_G_mat_csc;
    std::vector<irsolver::Node*> m_G_mat_nodes;
    std::vector<NodeMap> m_layer_maps;
    int m_numC4{1}; //TODO get from somewhere.

    double m_getConductance(NodeIdx t_row, NodeIdx t_col) {
        if(m_G_mat_dok.num_cols<=t_col||m_G_mat_dok.num_rows<=t_row ){
            cout<<"ERROR: Index out of bound for getting G matrix conductance. Ensure object is intialized to the correct size first.\n";
            exit(1);
        }
        GMatLoc key = make_pair(t_col,t_row); 
        map<GMatLoc,double>::iterator it = m_G_mat_dok.values.find( key ); 
        if (it != m_G_mat_dok.values.end()){
            return it->second;
        } else {
            return 0;
        }
    }
    void m_setConductance(NodeIdx t_row, NodeIdx t_col, double t_cond) {
        if(m_G_mat_dok.num_cols<=t_col||m_G_mat_dok.num_rows<=t_row ){
            cout<<"ERROR: Index out of bound for setting G matrix conductance. Ensure object is intialized to the correct size first.\n";
            exit(1);
        }
        GMatLoc key = make_pair(t_col,t_row); 
        m_G_mat_dok.values[key] = t_cond;
    }

    Node* m_nearestYNode(NodeMap::iterator x_itr,int t_y) {
        map<int,Node*> &y_map = x_itr->second;
        map<int,Node*>::iterator y_itr = y_map.lower_bound(t_y);
        if(y_map.size() == 1 || y_itr==y_map.end() || y_itr==y_map.begin() ) {
            if(y_map.size() == 1 ){
                y_itr = y_map.begin();
            } else if (y_itr==y_map.end()) {
                y_itr = prev(y_itr);
            } else {
            }
            return y_itr->second;
        } else {
            map<int,Node*>::iterator y_prev;
            y_prev = prev(y_itr);
            int dist1 = abs(y_prev->first - t_y);
            int dist2 = abs(y_itr->first - t_y);
            if(dist1<dist2) {
                return y_prev->second;
            } else {
                return y_itr->second;
            }
        }
    }
    double m_getConductivity(double width,double length,double rho){
        if (0>=length || 0>= width || 0>=rho) {
            return 0.0;
        } else {
            return width/(rho*length);
        }
    }

 };

class IRSolver
{
public:
    IRSolver(dbDatabase* t_db)
    {
     //constructor   
        m_db = t_db;
        m_readC4Data();
        //cout<<" c4 "<<endl;
        m_createGmat();
        //cout<<" gmat "<<endl;
        m_createJ();
        //cout<<" J "<<endl;
        addC4Bump();
        //cout<<" bump "<<endl;
        m_Gmat->generateCSCMatrix();
    }
    ~IRSolver()
    {
        delete m_Gmat;
    }
    irsolver::GMat* getGMat() {
        return m_Gmat;
    }
    void setNodeCurrent(){
    }
    void addC4Bump(){ //TODO temporary where to set it from? 
        //nodes 1 5 9 13 m7 via nodes //temp at 9
        m_Gmat -> addC4Bump(9,0); //add the 0th bump
        m_J.push_back(1.1);//push back first vdd
    }
    vector<double> getJ(){
        return m_J;
    }
private:
    dbDatabase* m_db;
    GMat* m_Gmat;
    int m_node_density{2800};//TODO get from somehwere
    vector<double> m_J;
//    C4Bump m_c4_bump;
    
    void m_readC4Data() {
        
    }
    void m_createJ(){ //take current_map as an input? 
        m_J.resize(m_Gmat->getNumNodes(),0);             
    }

    void m_createGmat()
    {
        std::vector<Node*> node_vector;

        //sanity check
        dbTech* tech = m_db->getTech();
        dbSet<dbTechLayer> layers = tech->getLayers();
        dbSet<dbTechLayer>::iterator litr;
        //for( litr = layers.begin(); litr != layers.end(); ++litr ) {
        //    dbTechLayer* layer = *litr;
        //    std::cout<<"Layer: "<<layer->getName()<<endl;
        //}
        int num_routing_layers = tech->getRoutingLayerCount();
        //#std::map<double,map<double,irsolver::Node*>>*
        m_Gmat = new GMat(num_routing_layers);
        dbChip* chip = m_db->getChip();
        dbBlock* block = chip->getBlock();
        dbSet<dbNet> nets = block->getNets();
        std::vector<dbNet*> vdd_nets;
        std::vector<dbNet*> gnd_nets;
        int netCNT = nets.size();

        dbSet<dbNet>::iterator nIter;
        for(nIter = nets.begin(); nIter != nets.end(); ++nIter) {
            dbNet* curDnet = *nIter;
            
            dbSigType nType = curDnet->getSigType();
            if( nType == dbSigType::GROUND ) {
                gnd_nets.push_back(curDnet); 
                //std::cout<<"Ground net found \n";
            } else if( nType == dbSigType::POWER) {
                vdd_nets.push_back(curDnet); 
                //std::cout<<"VDD net found \n";
            } else {
                continue;
            } 
        } 
        //TODO for lower layers insert regular grid nodes;
        std::vector<dbNet*>::iterator vIter;
        for(vIter = vdd_nets.begin(); vIter != vdd_nets.end(); ++vIter) {//only 1 is expected?
            dbNet* curDnet = *vIter;
            dbSet<dbSWire> swires = curDnet->getSWires();
            dbSet<dbSWire>::iterator sIter;
            for(sIter = swires.begin(); sIter != swires.end(); ++sIter) {//only 1 is expected?
                dbSWire* curSWire = *sIter;
                dbSet<dbSBox> wires = curSWire->getWires();
                dbSet<dbSBox>::iterator wIter;
                for(wIter = wires.begin(); wIter != wires.end(); ++wIter) {
                    dbSBox* curWire = *wIter;
                    if( curWire->isVia()){
                        dbVia* via = curWire->getBlockVia();
                        dbBox* via_bBox = via->getBBox();
                        BBox bBox=make_pair((via_bBox->getDX())/2,(via_bBox->getDY())/2);
                        int x,y;
                        curWire->getViaXY(x,y);
                        dbTechLayer* via_layer = via->getBottomLayer();
                        int l = via_layer->getRoutingLevel();
                        //cout<<"node l,x,y bbox : "<<x<<" "<<y<<" "<<l<<" "<<bBox.first<<" "<<bBox.second<< " \n";
                        if(1 != l ){ //layer 1 nodes handled separately with a fine grid 
                            m_Gmat->setNode(x, y, l, bBox) ;
                        }
                        via_layer = via->getTopLayer();
                        l = via_layer->getRoutingLevel();
                        //cout<<"node l,x,y bbox : "<<x<<" "<<y<<" "<<l<<" "<<bBox.first<<" "<<bBox.second<< " \n";
                        m_Gmat->setNode(x, y, l, bBox) ;
                    } else {
                        int  x_loc1, x_loc2, y_loc1, y_loc2;
                        dbTechLayer* wire_layer = curWire->getTechLayer();
                        int l = wire_layer->getRoutingLevel();
                        dbTechLayerDir::Value layer_dir = wire_layer->getDirection() ;
                        if(layer_dir == dbTechLayerDir::Value::HORIZONTAL) {
                            y_loc1 = (curWire->yMin()+curWire->yMax())/2;
                            y_loc2 = (curWire->yMin()+curWire->yMax())/2;
                            x_loc1 = curWire->xMin();
                            x_loc2 = curWire->xMax();
                        } else {
                            x_loc1 = (curWire->xMin()+curWire->xMax())/2;
                            x_loc2 = (curWire->xMin()+curWire->xMax())/2;
                            y_loc1 = curWire->yMin();
                            y_loc2 = curWire->yMax();
                        }
                        if(l == 1) { //special case for layer 1 we design a dense grid
                            if(layer_dir == dbTechLayerDir::Value::HORIZONTAL){
                                int x_i;
                                for(x_i=x_loc1;x_i<=x_loc2;x_i=x_i+m_node_density) {
                                    m_Gmat->setNode(x_i, y_loc1, l, make_pair(0,0)) ;                                       
                                }
                            } else {
                                int y_i;
                                for(y_i=y_loc1;y_i<=y_loc2;y_i=y_i+m_node_density) {
                                    m_Gmat->setNode(x_loc1, y_i, l, make_pair(0,0)) ;                                       
                                }
                            }
                        } else { //add end nodes
                            m_Gmat->setNode(x_loc1, y_loc1, l, make_pair(0,0)) ;
                            m_Gmat->setNode(x_loc2, y_loc2, l, make_pair(0,0)) ;
                        }
                    }
                }
            }
        }
                        
        // All new nodes must be inserted by this point
        //initialize G Matrix
        cout<<" number of nodes "<<m_Gmat->getNumNodes()<<endl;
        m_Gmat->initializeGmatDok();
        for(vIter = vdd_nets.begin(); vIter != vdd_nets.end(); ++vIter) {//only 1 is expected?
            //std::cout<<"Operating on VDD net"<<endl;
            dbNet* curDnet = *vIter;
            dbSet<dbSWire> swires = curDnet->getSWires();
            dbSet<dbSWire>::iterator sIter;
            for(sIter = swires.begin(); sIter != swires.end(); ++sIter) {//only 1 is expected?
                //std::cout<<"Operating on VDD net Swire"<<endl;
                dbSWire* curSWire = *sIter;
                dbSet<dbSBox> wires = curSWire->getWires();
                dbSet<dbSBox>::iterator wIter;
                for(wIter = wires.begin(); wIter != wires.end(); ++wIter) {
                    dbSBox* curWire = *wIter;
                    if( curWire->isVia()){
                        //cout<<"1 ";
                        //fflush(stdout);
                        dbVia* via = curWire->getBlockVia();
                        dbBox* via_bBox = via->getBBox();
                        BBox bBox=make_pair((via_bBox->getDX())/2,(via_bBox->getDY())/2);
                        //cout<<"2 ";
                        //fflush(stdout);
                        int x,y;
                        curWire->getViaXY(x,y);
                        dbTechLayer* via_layer = via->getBottomLayer();
                        //cout<<"3 ";
                        //fflush(stdout);
                        int l = via_layer->getRoutingLevel();
                        int l_bot =l;

                        // //TODO assuming default if zero
                        double R  = via_layer->getUpperLayer()->getResistance();
                        //cout<<"4 "<<endl;
                        if(R<=0.01){
                            R=1.0;
                        }
                        //cout<<"node l,x,y : "<<x<<" "<<y<<" "<<l_bot<<endl;
                        Node* node_bot = m_Gmat->getNode(x, y, l) ;

                        via_layer = via->getTopLayer();
                        l = via_layer->getRoutingLevel();
                        int l_top =l;
                        //cout<<"node l,x,y : "<<x<<" "<<y<<" "<<l_top<<endl;
                        Node* node_top = m_Gmat->getNode(x, y, l) ;
                        //cout<<"via"<<endl;
                        if(node_bot == nullptr || node_top == nullptr) {
                            cout<<"ERROR: null pointer received for expected node. Code may fail ahead.\n";
                            exit(1);
                        } else {
                            //cout<<"setting via ";
                            m_Gmat->setConductance(node_bot,node_top,1/R);
                            //cout<<" set via"<<endl;
                        }
                    } else {
                        //std::cout<<"Operating on Other"<<endl;
                        dbTechLayer* wire_layer = curWire->getTechLayer();
                        double rho = wire_layer->getResistance() * double(wire_layer->getWidth())/double((wire_layer->getTech())->getDbUnitsPerMicron()); 
                        dbTechLayerDir::Value layer_dir = wire_layer->getDirection() ;
                        m_Gmat->generateStripeConductance(wire_layer->getRoutingLevel(),layer_dir,curWire->xMin(),curWire->xMax(), curWire->yMin(),curWire->yMax(),rho);
                        //cout<<"added stripe"<<endl;
                    }
                    //cout<<"added wire"<<endl;
                }
                //cout<<"added swire"<<endl;
            }
            //cout<<"added vdd"<<endl;
        }
    }
};


};
dbDatabase*  import_db(const char* dbLoc) {
    //  odb::dbDatabase* db = NULL;
      dbDatabase* db_h = new dbDatabase();
      dbDatabase* db = db_h->create();
    
      FILE* fp = fopen(dbLoc, "rb");
      if( fp == NULL ) {
        std::cout << "ERROR: Can't open " <<  dbLoc << endl;
        exit(1);
      }
      db->read(fp);
      fclose(fp);
      int db_id = db->getId(); 
      return db;
    }


void test_solving( irsolver::CscMatrix& t_G_mat,vector<double>& t_J) {
    SuperMatrix A, L, U, B;
    SuperLUStat_t stat;
    superlu_options_t options;
    int nrhs = 1;
    int m = t_G_mat.num_rows;
    int n = t_G_mat.num_cols;
    int info;
    //cout<<"GMAT size "<<t_G_mat.size()<<endl;
    
    int      *perm_r; /* row permutations from partial pivoting */
    int      *perm_c; /* column permutation vector */
    double *rhs = &t_J[0];
    double *end = rhs+m;
    for(double *i = rhs; i!=end; ++i){
        cout<<*i<<endl;
    }
    cout<<"size J"<<t_J.size()<<endl;
    cout<<"1 ";
    fflush(stdout);
    //dCreate_Dense_Matrix(&A, m, n, G_mat, m, SLU_DN, SLU_D, SLU_GE);
    //if ( !(rhs = doubleMalloc(m * nrhs)) ) ABORT("Malloc fails for rhs[].");
    cout<<"2 ";
    fflush(stdout);
    dCreate_Dense_Matrix(&B, m, nrhs, rhs, m, SLU_DN, SLU_D, SLU_GE);
    cout<<"3 ";
    fflush(stdout);
    dPrint_Dense_Matrix("B", &B);
   
    int nnz =  t_G_mat.nnz;
    double* values = &t_G_mat.values[0];
    int* row_idx = &t_G_mat.row_idx[0];
    int* col_ptr = &t_G_mat.col_ptr[0];

    dCreate_CompCol_Matrix(&A, m, n, nnz, values, row_idx, col_ptr, SLU_NC, SLU_D, SLU_GE);
    if ( !(perm_r = intMalloc(m)) ) ABORT("Malloc fails for perm_r[].");
    if ( !(perm_c = intMalloc(n)) ) ABORT("Malloc fails for perm_c[].");
    /* Set the default input options. */
    set_default_options(&options);
    options.ColPerm = NATURAL;
    cout<<"4 ";
    fflush(stdout);

    /* Initialize the statistics variables. */
    StatInit(&stat);
    cout<<"5 ";
    fflush(stdout);
    /* Solve the linear system. */
    dgssv(&options, &A, perm_c, perm_r, &L, &U, &B, &stat, &info);
    cout<<"6 ";
    fflush(stdout);
    
    dPrint_CompCol_Matrix("A", &A);
    //dPrint_CompCol_Matrix("U", &U);
    //dPrint_SuperNode_Matrix("L", &L);
    dPrint_Dense_Matrix("B", &B);
    //print_int_vec("\nperm_r", m, perm_r);
    cout<<"7 ";
    fflush(stdout);
    ofstream myfile;
    myfile.open ("V_mat.csv");
    DNformat     *Bstore = (DNformat *) B.Store;
    register int i, j, lda = Bstore->lda;
    double       *dp;
    dp = (double *) Bstore->nzval;
    for (j = 0; j < B.ncol; ++j) {
        for (i = 0; i < B.nrow; ++i) 
            myfile<<setprecision(10)<<dp[i + j*lda]<<",";
        myfile<<"\n";
    }
    myfile.close();

    /* De-allocate storage */
    //SUPERLU_FREE (rhs);
    SUPERLU_FREE (perm_r);
    SUPERLU_FREE (perm_c);
    Destroy_SuperMatrix_Store(&A);
    Destroy_SuperMatrix_Store(&B);
    Destroy_SuperNode_Matrix(&L);
    Destroy_CompCol_Matrix(&U);
    StatFree(&stat);
    cout<<"8 "<<endl;
}

int main() {
    //vector<irsolver::Node*> test;
    //cout<<test.max_size();
    cout<<"test\n";
    irsolver::GMat* gmat_obj;
    irsolver::Node* node;
    std::cout<<"GMat test code \n";
    gmat_obj = new irsolver::GMat(7);
    std::cout<<"New_obj\n";
    node = new irsolver::Node();
    node->setLoc(3,4,5);
    gmat_obj->insertNode(node);
    node = new irsolver::Node();
    node->setLoc(3,2,3);
    gmat_obj->insertNode(node);
    node = new irsolver::Node();
    node->setLoc(1,4,6);
    gmat_obj->insertNode(node);
    std::cout<<"New_objs inserted\n";
    //gmat_obj->print();
    
    //dbDatabase* db = import_db("/home/sachin00/chhab011/PDNA/aes_pdn.db");
    //dbDatabase* db = import_db("/project/parhi-group00/unnik005/temp/PDNA/aes_pdn.db");
    dbDatabase* db = import_db("/project/parhi-group00/unnik005/temp/PDN.db");
    irsolver::IRSolver* irsolve_h = new irsolver::IRSolver(db);
    gmat_obj = irsolve_h->getGMat();
    //gmat_obj->print();
    irsolver::CscMatrix& test_gmat = *(gmat_obj->getGMat());
    ofstream myfile;
    int vsize;
    //TODO for debug write it from csc format or is it too big?
    //myfile.open ("G_mat.csv");
    //vsize = test_gmat.size();
    //for (int n=0; n<vsize; n++)
    //{
    //    if(n%1877 ==1876){
    //    myfile <<  std::setprecision(10)<<test_gmat[n]<<"\n";
    //    }else{
    //    myfile <<  std::setprecision(10)<<test_gmat[n] <<",";
    //    }
    //}
    //myfile.close();
    vector<double> test_J = irsolve_h->getJ();//we need a copy as superlu destros that loc
    myfile.open ("J.csv");
    vsize = test_J.size();
    for (int n=0; n<vsize; n++)
    {
        myfile <<  std::setprecision(10)<<test_J[n] <<"\n";
    }

    myfile.close();
    test_solving(test_gmat,test_J);
    return 1;

}

