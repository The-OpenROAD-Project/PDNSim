#include <vector>
#include <iostream>
#include "gmat.h"
#include "node.h"

using namespace std;

Node* GMat::GetNode(NodeIdx t_node) {
    if( 0 <= t_node && m_n_nodes > t_node) {
        return m_G_mat_nodes[t_node];
    } else {
        return nullptr;
    }
}
Node* GMat::GetNode(int t_x, int t_y, int t_l) {
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
            NodeLoc node1_loc = node1->GetLoc();
            NodeLoc node2_loc = node2->GetLoc();
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
void GMat::SetNode(NodeIdx t_node_loc, Node* t_node) {
    if( 0 <= t_node_loc && m_n_nodes > t_node_loc) {
        t_node->SetGLoc(t_node_loc);
        m_G_mat_nodes[t_node_loc] = t_node;
    }
}
void GMat::InsertNode(Node* t_node) {
    t_node->SetGLoc(m_n_nodes);
    int layer = t_node->GetLayerNum();
    NodeLoc nodeLoc = t_node->GetLoc();
    NodeMap& layer_map = m_layer_maps[layer];
    layer_map[nodeLoc.first][nodeLoc.second] = t_node;
    m_G_mat_nodes.push_back(t_node);
    m_n_nodes++;
}

Node* GMat::SetNode(int t_x, int t_y, int t_layer, BBox t_bBox) {
    NodeMap& layer_map = m_layer_maps[t_layer];
    //cout<<"set node params l,x,y bbox"<<t_x<<" "<<t_y<<" "<<t_layer<<" "<<t_bBox.first<<" "<<t_bBox.second<< " \n";
    if(layer_map.empty()){
        
        Node* node = new Node();
        node->SetLoc(t_x,t_y,t_layer);
        node->UpdateMaxBbox(t_bBox.first,t_bBox.second);
        InsertNode(node);
        //cout<<"layer empty inserting node\n";
        //node->print();
        return(node);
    }
    NodeMap::iterator x_itr = layer_map.find(t_x);
    if(x_itr!=layer_map.end()) {
        map<int,Node*>::iterator y_itr = x_itr->second.find(t_y);
        if(y_itr != x_itr->second.end()){
            Node* node = y_itr->second;
            node->UpdateMaxBbox(t_bBox.first,t_bBox.second);
            //cout<<"Node exists\n";
            return(node);
        }else{
            Node* node = new Node();
            node->SetLoc(t_x,t_y,t_layer);
            node->UpdateMaxBbox(t_bBox.first,t_bBox.second);
            InsertNode(node);
            //cout<<"Creating Node new y\n";
            return(node);
        }
        
    } else {
        Node* node = new Node();
        node->SetLoc(t_x,t_y,t_layer);
        node->UpdateMaxBbox(t_bBox.first,t_bBox.second);
        InsertNode(node);
        //cout<<"Creating Node new x\n";
        return(node);
    }
}

void GMat::print(){
    std::cout<<"GMat obj, with "<<m_n_nodes<<" nodes\n";
    for(NodeIdx i =0; i<m_n_nodes; i++) {
        Node* node_ptr = m_G_mat_nodes[i];
        if( node_ptr != nullptr) {
            node_ptr->print();
        }
    }
}
void GMat::SetConductance(Node* t_node1, Node* t_node2, double t_cond) {
    //cout<<"1";
    //fflush(stdout);
    NodeIdx node1_r = t_node1->GetGLoc();
    NodeIdx node2_r = t_node2->GetGLoc();
    //cout<<"2 loc1 "<<node1_r<<" loc2 "<<node2_r<<" ";
    //fflush(stdout);
    double node11_cond = GetConductance(node1_r,node1_r);
    double node22_cond = GetConductance(node2_r,node2_r);
    double node12_cond = GetConductance(node1_r,node2_r);
    double node21_cond = GetConductance(node2_r,node1_r);
    //cout<<"3";
    //fflush(stdout);
    m_setConductance(node1_r,node1_r,node11_cond+t_cond);
    m_setConductance(node2_r,node2_r,node22_cond+t_cond);
    m_setConductance(node1_r,node2_r,node12_cond-t_cond);
    m_setConductance(node2_r,node1_r,node21_cond-t_cond);
    //cout<<"4";
    //fflush(stdout);
}
void GMat::InitializeGmatDok(){
    if(m_n_nodes<=0) {
       cout<<"ERROR: no nodes in object initialization stopped.\n"; 
       exit(1);
    }else{
        m_G_mat_dok.num_cols = m_n_nodes+m_numC4;
        m_G_mat_dok.num_rows = m_n_nodes+m_numC4;
    }
    //m_G_mat.assign((m_n_nodes+m_numC4)*( m_n_nodes+m_numC4),0);
}
NodeIdx GMat::GetNumNodes() {//debug
    return m_n_nodes;
}
CscMatrix* GMat::GetGMat() {//Nodes debug
    return &m_G_mat_csc;
}

void GMat::GenerateStripeConductance(int t_l, odb::dbTechLayerDir::Value layer_dir, int t_x_min, int t_x_max, int t_y_min, int t_y_max,double t_rho){
    NodeMap &layer_map = m_layer_maps[t_l];
    //cout<<"RHO "<<t_rho<<endl;
    if(layer_dir == odb::dbTechLayerDir::Value::HORIZONTAL){
        NodeMap::iterator x_itr;
        NodeMap::iterator x_prev;
        int y_loc = (t_y_min+t_y_max)/2;
        int i = 0;
        for( x_itr=layer_map.lower_bound(t_x_min); x_itr->first <= t_x_max && x_itr!=layer_map.end(); ++x_itr){
            if((x_itr->second).find(y_loc) == (x_itr->second).end()){
                continue;
            }
            if(i == 0 ){
                i=1;
            } else {
                //if(t_l!=1) cout<<"x1 "<<x_itr->first<<" y "<<y_loc<<endl;
                Node* node1 = (x_itr->second).at(y_loc);                    
                //if(t_l!=1) cout<<"x1 "<<x_prev->first<<" y "<<y_loc<<endl;
                Node* node2 = (x_prev->second).at(y_loc);
                int width = t_y_max-t_y_min;
                int length = x_itr->first-x_prev->first;
                double cond = m_getConductivity(width,length,t_rho);
                //if(t_l!=1) cout<<"layer hor "<<t_l<<" cond "<<cond<<endl;
                SetConductance(node1,node2,cond);
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
                SetConductance(node1,node2,cond);
            }
            y_prev = y_itr;
        }
    }
    
}
void GMat::AddC4Bump(int t_loc,int t_C4Num){
    m_setConductance(t_loc, t_C4Num+m_n_nodes,1);     
    m_setConductance(t_C4Num+m_n_nodes,t_loc,1);     
}
void GMat::GenerateCSCMatrix(){
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
    m_G_mat_csc.col_ptr.push_back(m_G_mat_csc.nnz);
    //temp_compare_gmat();
}



double GMat::GetConductance(NodeIdx t_row, NodeIdx t_col) {
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
void GMat::m_setConductance(NodeIdx t_row, NodeIdx t_col, double t_cond) {
    if(m_G_mat_dok.num_cols<=t_col||m_G_mat_dok.num_rows<=t_row ){
        cout<<"ERROR: Index out of bound for setting G matrix conductance. Ensure object is intialized to the correct size first.\n";
        exit(1);
    }
    GMatLoc key = make_pair(t_col,t_row); 
    m_G_mat_dok.values[key] = t_cond;
    //m_G_mat[( m_n_nodes+m_numC4)*t_col+t_row] =t_cond;
}

Node* GMat::m_nearestYNode(NodeMap::iterator x_itr,int t_y) {
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
double GMat::m_getConductivity(double width,double length,double rho){
    if (0>=length || 0>= width || 0>=rho) {
        return 0.0;
    } else {
        return width/(rho*length);
    }
}

std::vector<Node*> GMat::getNodes() {
        return m_G_mat_nodes;
    }

