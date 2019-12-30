#ifndef __IRSOLVER_IRSOLVER_
#define __IRSOLVER_IRSOLVER_

#include "gmat.h"
#include "db.h"

class IRSolver {
public:
    IRSolver(odb::dbDatabase* t_db)
    {
        m_db = t_db;
        m_readC4Data();
        m_createGmat();
        m_createJ();
        m_addC4Bump();
        m_Gmat->GenerateCSCMatrix();
    }
    ~IRSolver()
    {
        delete m_Gmat;
    }
    GMat* GetGMat();
    std::vector<double> getJ();
    void solve_ir();
private:
    odb::dbDatabase* m_db;
    GMat* m_Gmat;
    int m_node_density{2800};//TODO get from somehwere
    std::vector<double> m_J;
    std::vector<std::tuple<int,int,int,double>> m_C4Bumps;
    std::vector<NodeIdx> m_C4GLoc;
    void m_addC4Bump();
    void m_readC4Data();
    void m_createJ();
    void m_createGmat();
};
#endif
