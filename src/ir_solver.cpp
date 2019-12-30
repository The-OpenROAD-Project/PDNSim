#include <vector>
#include <math.h>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <map>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <iterator>
#include <string>

#include "slu_ddefs.h"
#include "db.h"
#include "ir_solver.h"
#include "node.h"
#include "gmat.h"

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


GMat* IRSolver::GetGMat() {
    return m_Gmat;
}

vector<double> IRSolver::getJ(){
    return m_J;
}

void IRSolver::solve_ir() {
    SuperMatrix A, L, U, B;
    SuperLUStat_t stat;
    superlu_options_t options;
    int nrhs = 1;
    CscMatrix* Gmat = m_Gmat->GetGMat();
    int m = Gmat->num_rows;
    int n = Gmat->num_cols;
    int info;
    int      *perm_r; /* row permutations from partial pivoting */
    int      *perm_c; /* column permutation vector */
    vector<double> J = getJ();
    double *rhs = &J[0];
    dCreate_Dense_Matrix(&B, m, nrhs, rhs, m, SLU_DN, SLU_D, SLU_GE);
    int nnz =  Gmat->nnz;
    double* values = &(Gmat->values[0]);
    int* row_idx = &(Gmat->row_idx[0]);
    int* col_ptr = &(Gmat->col_ptr[0]);
    dCreate_CompCol_Matrix(&A, m, n, nnz, values, row_idx, col_ptr, SLU_NC, SLU_D, SLU_GE);
    if ( !(perm_r = intMalloc(m)) ) ABORT("Malloc fails for perm_r[].");
    if ( !(perm_c = intMalloc(n)) ) ABORT("Malloc fails for perm_c[].");
    set_default_options(&options);
    options.ColPerm = NATURAL;
    /* Initialize the statistics variables. */
    StatInit(&stat);
    dPrint_CompCol_Matrix("A", &A);
    cout<<"just starting solving"<<endl;

    /* Solve the linear system. */
    dgssv(&options, &A, perm_c, perm_r, &L, &U, &B, &stat, &info);
    cout<<"just finished solving"<<endl;
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

    //TODO keep copies fo LU for later?
    /* De-allocate storage */
    //SUPERLU_FREE (rhs);
    SUPERLU_FREE (perm_r);
    SUPERLU_FREE (perm_c);
    Destroy_SuperMatrix_Store(&A);
    Destroy_SuperMatrix_Store(&B);
    Destroy_SuperNode_Matrix(&L);
    Destroy_CompCol_Matrix(&U);
    StatFree(&stat);
}

void IRSolver::m_addC4Bump(){
    for(int it = 0; it<m_C4Bumps.size(); ++it){
        double voltage = get<3>(m_C4Bumps[it]);            
        NodeIdx node_loc = m_C4GLoc[it];
        m_Gmat -> AddC4Bump(node_loc,it); //add the 0th bump
        m_J.push_back(voltage);//push back first vdd
    }
}

void  IRSolver::m_readC4Data() {
    std::ifstream file("../c4BumpLoc.txt_gcd"); //TODO read file as an input 
    std::string line = "";
    // Iterate through each line and split the content using delimeter
    while (getline(file, line))
    {
        tuple<int,int,int,double> c4_bump;
        int first, second, layer;
        double voltage;
        stringstream X(line);
        string val;
        for(int i =0; i<4;++i){
            getline(X,val,',');
            if(i==0){
                first = stoi(val);
            } else if(i ==1) {
                second = stoi(val);
            } else if(i ==2) {
                layer = stoi(val);
            } else{
                voltage = stod(val);
            }
        }
        cout<<"location "<<first<<" "<<second<<" "<<layer<<" value "<<voltage<<endl;
    	m_C4Bumps.push_back(make_tuple(first,second,layer,voltage));
    }
    file.close();

}
void  IRSolver::m_createJ(){ //take current_map as an input? 
    m_J.resize(m_Gmat->GetNumNodes(),0);             
    //TODO temp making for checking
    for(int i = 195; i<1875;i++)
        m_J[i]=-1e-6;
}

void  IRSolver::m_createGmat()
{
    std::vector<Node*> node_vector;

    //sanity check
    dbTech* tech = m_db->getTech();
    dbSet<dbTechLayer> layers = tech->getLayers();
    dbSet<dbTechLayer>::iterator litr;
    int num_routing_layers = tech->getRoutingLayerCount();

    int num_c4 = m_C4Bumps.size();
    m_Gmat = new GMat(num_routing_layers,num_c4);
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
        } else if( nType == dbSigType::POWER) {
            vdd_nets.push_back(curDnet); 
        } else {
            continue;
        } 
    } 
    std::vector<dbNet*>::iterator vIter;
    for(vIter = vdd_nets.begin(); vIter != vdd_nets.end(); ++vIter) {
        dbNet* curDnet = *vIter;
        dbSet<dbSWire> swires = curDnet->getSWires();
        dbSet<dbSWire>::iterator sIter;
        for(sIter = swires.begin(); sIter != swires.end(); ++sIter) {
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
                    if(1 != l ){  
                        m_Gmat->SetNode(x, y, l, bBox) ;
                    }
                    via_layer = via->getTopLayer();
                    l = via_layer->getRoutingLevel();
                    m_Gmat->SetNode(x, y, l, bBox) ;
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
                                m_Gmat->SetNode(x_i, y_loc1, l, make_pair(0,0)) ;                                       
                            }
                        } else {
                            int y_i;
                            for(y_i=y_loc1;y_i<=y_loc2;y_i=y_i+m_node_density) {
                                m_Gmat->SetNode(x_loc1, y_i, l, make_pair(0,0)) ;                                       
                            }
                        }
                    } else { //add end nodes
                        m_Gmat->SetNode(x_loc1, y_loc1, l, make_pair(0,0)) ;
                        m_Gmat->SetNode(x_loc2, y_loc2, l, make_pair(0,0)) ;
                    }
                }
            }
        }
    }
    //insert c4 bumps as nodes
    for(int it = 0; it<m_C4Bumps.size(); ++it){
        int x = get<0>(m_C4Bumps[it]);            
        int y = get<1>(m_C4Bumps[it]);    
        int l = get<2>(m_C4Bumps[it]);    
        Node* node = m_Gmat->SetNode(x,y,l,make_pair(0,0));
        m_C4GLoc.push_back(node->GetGLoc());
    }
                    
    // All new nodes must be inserted by this point
    //initialize G Matrix
    cout<<" number of nodes "<<m_Gmat->GetNumNodes()<<endl;
    m_Gmat->InitializeGmatDok();
    m_Gmat->print();
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
                    dbVia* via = curWire->getBlockVia();
                    dbBox* via_bBox = via->getBBox();
                    BBox bBox=make_pair((via_bBox->getDX())/2,(via_bBox->getDY())/2);
                    int x,y;
                    curWire->getViaXY(x,y);
                    dbTechLayer* via_layer = via->getBottomLayer();
                    int l = via_layer->getRoutingLevel();
                    int l_bot =l;

                    // //TODO assuming default if zero
                    double R  = via_layer->getUpperLayer()->getResistance();
                    if(R<=0.01){
                        R=1.0;
                    }
                    Node* node_bot = m_Gmat->GetNode(x, y, l) ;

                    via_layer = via->getTopLayer();
                    l = via_layer->getRoutingLevel();
                    int l_top =l;
                    Node* node_top = m_Gmat->GetNode(x, y, l) ;
                    if(node_bot == nullptr || node_top == nullptr) {
                        cout<<"ERROR: null pointer received for expected node. Code may fail ahead.\n";
                        exit(1);
                    } else {
                        m_Gmat->SetConductance(node_bot,node_top,1/R);
                    }
                } else {
                    dbTechLayer* wire_layer = curWire->getTechLayer();
                    double rho = wire_layer->getResistance() * double(wire_layer->getWidth())/double((wire_layer->getTech())->getDbUnitsPerMicron()); 
                    dbTechLayerDir::Value layer_dir = wire_layer->getDirection() ;
                    m_Gmat->GenerateStripeConductance(wire_layer->getRoutingLevel(),layer_dir,curWire->xMin(),curWire->xMax(), curWire->yMin(),curWire->yMax(),rho);
                }
            }
        }
    }
}

