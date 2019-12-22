#include <vector>
#include "db.h"
#include <bits/stdc++.h> 
#include <math.h>
#include <iostream>

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
typedef std::pair<int,int> BBox;
typedef std::pair<int,int> NodeLoc;

//TODO convert to all int float keys have a problem.
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
    int getGLoc() {
        return m_node_loc;
    }
    bool setGLoc(int t_loc) {
        m_node_loc =t_loc;
        //TODO should we insert checks? or make it void
        return true;
    }
    void print(){
        cout <<"Node: "<<m_node_loc<<"\n";
        cout <<"    location: Layer "<<m_layer<<", x "<<m_loc.first<<", y "<<m_loc.second<<"\n"; 
        cout <<"    Bounding box: x "<<m_bBox.first<<", y "<<m_bBox.second<<"\n"; 
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
        //cout<<"vals x "<<t_nodeLoc.first<<" "<< nodeLoc.first<<"\n";
        //cout<<"vals y "<<t_nodeLoc.second<<" "<< nodeLoc.second<<"\n";
        int DX = abs(t_nodeLoc.first - nodeLoc.first);
        int DY = abs(t_nodeLoc.second - nodeLoc.second);
        //cout<<"distances DX "<<DX<<" DY"<<DY<<" box1 "<<t_bBox.first <<" "<<t_bBox.second  <<" box 2"<<nodeBbox.first <<" "<< nodeBbox.second<<"\n";
        bool cond =( (DX<=t_bBox.first || DX <= nodeBbox.first) 
               &&(DY<=t_bBox.second || DY <= nodeBbox.second) );
        if(cond) {
            t_dist = DX+DY;
        }
            return cond;
        
    }
    void updateMaxBbox(int t_dX,int t_dY) {
        NodeLoc nodeLoc = m_loc;
        BBox nodeBbox = m_bBox;
        int DX = max(nodeBbox.first ,t_dX);
        int DY = max(nodeBbox.second,t_dY);
        setBbox(DX,DY);
    }

private:
    int m_layer ;
    NodeLoc m_loc; //layer,x,y
    int m_node_loc{0} ;
    BBox m_bBox;
};

typedef std::map<int,map<int,Node*>> NodeMap;

class GMat
{
public:
    GMat(int t_num_layers)
        :m_num_layers(t_num_layers),m_layer_maps(t_num_layer,NodeMap())
    {
    }
    ~GMat()
    {
        //proper destructor code requried
        //delete all nodes pointeres in map
    }
    Node* getNode(int t_node) {
        if( 0 <= t_node && m_n_nodes > t_node) {
            return m_G_mat_nodes[t_node];
        } else {
            return nullptr;
        }
    }
    Node* getNode(int t_node_loc) {
        if( 0 <= t_node_loc && m_n_nodes > t_node_loc) {
            return m_G_mat_nodes[t_node_loc]
        } else {
            return nullptr;
        }
    }
    Node* getNode(int t_x, int t_y, int t_l) {
        NodeMap &layer_map = m_layer_maps[t_l-1];
        NodeMap::iterator it_x = layer_map.find(t_x);
        if(it_x == layer_map.end()) {
            return nullptr;
        } else {
            map<int,Node*>::iterator it_y = it_x->second.find(t_y);
            if(it_y == it_x->second.end())  {
                return nullptr;
            } else {
                return it_y->second;
            }
        }
    }
    void setNode(int t_node_loc, irsolver::Node* t_node) {
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
    ///TODO big issue need to also go to the previous / next column.
        NodeMap& layer_map = m_layer_maps[t_layer];
        NodeMap::iterator low, prev;
        NodeLoc check_node_loc;
        Node* node_low;
        Node* node_prev ;
        //cout<<"set node params l,x,y bbox"<<t_x<<" "<<t_y<<" "<<t_layer<<" "<<t_bBox.first<<" "<<t_bBox.second<< " \n";
        check_node_loc = make_pair(t_x,t_y);
        if(layer_map.empty()){
            
            Node* node = new Node();
            node->setLoc(t_x,t_y,t_layer);
            node->updateMaxBbox(t_bBox.first,t_bBox.second);
            insertNode(node);
            //cout<<"layer empty inserting node\n";
            //node->print();
            return(node);
        }
        low = layer_map.lower_bound({t_x,t_y});
        if(low == layer_map.end()) {
            prev = std::prev(low);
            low = std::prev(low);
        } else if (low == layer_map.begin()){
            prev = layer_map.lower_bound({t_x,t_y});
        } else {
            prev = std::prev(low);
        }
        node_prev = prev->second;
        //cout<<"prev node \n";
        //node_prev->print();
        int dist_prev;
        node_low = low->second;
        //cout<<"low node \n";
        //node_low->print();
        int dist_low;
        if ((node_prev->withinBoundingBox(check_node_loc,t_bBox,dist_prev)) 
            ||(node_low->withinBoundingBox(check_node_loc,t_bBox,dist_low))){
            if(dist_prev<dist_low) {
                node_prev->updateMaxBbox(t_bBox.first,t_bBox.second);
                return node_prev;
            } else {
                node_low->updateMaxBbox(t_bBox.first,t_bBox.second);
                return node_low;
            }
        } else {
            Node* node = new Node();
            node->setLoc(t_x,t_y,t_layer);
            node->updateMaxBbox(t_bBox.first,t_bBox.second);
            insertNode(node);
            return(node);
        }
    }

    void print(){
        std::cout<<"GMat obj, with "<<m_n_nodes<<" nodes\n";
        for(int i =0; i<m_n_nodes; i++) {
            Node* node_ptr = m_G_mat_nodes[i];
            if( node_ptr != nullptr) {
                node_ptr->print();
            }
        }
    }
    void setConductance(irsolver::Node* t_node1, irsolver::Node* t_node2, double t_cond) {
        int node1_r = t_node1->getGLoc();
        int node2_r = t_node2->getGLoc();
        double node11_cond = m_getConductance(node1_r,node1_r);
        double node22_cond = m_getConductance(node2_r,node2_r);
        double node12_cond = m_getConductance(node1_r,node2_r);
        double node21_cond = m_getConductance(node2_r,node1_r);
        m_setConductance(node1_r,node1_r,node11_cond+t_cond);
        m_setConductance(node2_r,node2_r,node22_cond+t_cond);
        m_setConductance(node1_r,node2_r,node12_cond-t_cond);
        m_setConductance(node2_r,node1_r,node21_cond-t_cond);
    }
    void initializeGmat(){
        if(m_n_nodes<=0) {
           cout<<"WARNING: no nodes in object initialization stopped.\n" 
        }else{
            m_G_mat.assign(m_n_nodes*m_n_nodes,0);
        }
    }
    
private:
    int m_n_nodes{0};
    int m_num_layers;
    std::vector<double> m_G_mat;
    std::vector<irsolver::Node*> m_G_mat_nodes;
    std::vector<NodeMap> m_layer_maps;
    bool m_sortByY(const pair<int,int> &a, 
              const pair<int,int> &b) 
    { 
        return ((a.second < b.second)||((a.second == b.second)&&(a.first<b.first))); 
    } 

    double m_getConductance(int t_x, int t_y) {
        return m_G_mat[t_y*m_n_nodes + t_x];
    }
    void m_setConductance(int t_x, int t_y, double t_cond) {
        m_G_mat[t_y*m_n_nodes + t_x] = t_cond;
    }

 };

class IRSolver
{
public:
    IRSolver(dbDatabase* t_db)
    {
     //constructor   
        m_db = t_db;
        m_createGmat();

    }
    ~IRSolver()
    {
        delete m_Gmat;
    }
    irsolver::GMat* getGMat() {
        return m_Gmat;
    }
    
private:
    int m_L1_spacing {0};
    dbDatabase* m_db;
    int m_db_id;
    GMat* m_Gmat;
    int m_node_density{2800};//TODO get from somehwere
    double getConductivity(double width,double length,double rho){
        if (0>=length || 0>= width || 0>=rho) {
            return 0.0;
        } else {
            return width/(rho*length);
        }
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
        //    std::cout<<"Layer: "<<layer->getName()<<"\n";
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
                std::cout<<"Ground net found \n";
            } else if( nType == dbSigType::POWER) {
                vdd_nets.push_back(curDnet); 
                std::cout<<"VDD net found \n";
            } else {
                continue;
            } 
        } 
        //TODO for lower layers insert regular grid nodes;
        std::vector<dbNet*>::iterator vIter;
        int num_nodes = 0;
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
                        int length = curWire->getLength();
                        int width  = curWire->getWidth();
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
                        if(l == 1) { //special case for 1 we design a dense grid
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
        m_Gmat->initializeGmat();

        for(vIter = vdd_nets.begin(); vIter != vdd_nets.end(); ++vIter) {//only 1 is expected?
            std::cout<<"Operating on VDD net\n";
            dbNet* curDnet = *vIter;
            dbSet<dbSWire> swires = curDnet->getSWires();
            dbSet<dbSWire>::iterator sIter;
            for(sIter = swires.begin(); sIter != swires.end(); ++sIter) {//only 1 is expected?
                std::cout<<"Operating on VDD net Swire\n";
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

                        // add via resistance
                        // //TODO assuming default if zero
                        float R  = via_layer->getUpperLayer()->getResistance();
                        if(R<=1.0){
                            R=10.0;
                        }
                        node_bot = m_Gmat->getNode(x, y, l) ;

                        via_layer = via->getTopLayer();
                        l = via_layer->getRoutingLevel();
                        node_top = m_Gmat->getNode(x, y, l) ;
                        if(node_bot == nullptr || node_top == nullptr) {
                            cout<<"WARNING: null pointer received for expected node. Code may fail ahead.\n";
                        }
                        m_Gmat->setConductance(node_bot,node_top,1/R);
                    }
                    else{
                        //std::cout<<"Operating on Other\n";
                    }
                    //    layer = wire.getTechLayer()
                    //    R = layer.getResistance()
                    //    length = wire.getLength()
                    //    width = wire.getWidth()
                    //    
                }
            }
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
        std::cout << "ERROR: Can't open " <<  dbLoc << "\n";
        exit(1);
      }
      db->read(fp);
      fclose(fp);
      int db_id = db->getId(); 
      return db;
    }


int main() {
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
    gmat_obj->print();
    
    dbDatabase* db = import_db("/home/sachin00/chhab011/PDNA/aes_pdn.db");
    irsolver::IRSolver* irsolve_h = new irsolver::IRSolver(db);
    gmat_obj = irsolve_h->getGMat();
    gmat_obj->print();
    
    return 1;

}
