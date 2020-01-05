#include "openira_external.h"
#include <tcl.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "db.h"
#include "ir_solver.h"
#include <string>
#include <vector>
#include "gmat.h"
#include "node.h"
#include "parameters.h"

using namespace std;


extern "C" {
int Irsolver_Init(Tcl_Interp *interp);
}

int OpeniraTclAppInit(Tcl_Interp *interp) {
        std::cout << " > Running OpenIRA in interactive mode.\n";
        
        Tcl_Init(interp);
        Irsolver_Init(interp);

        return TCL_OK;
}



int Openira(Parameters* parmsToOpenira) {
    GMat* gmat_obj;
    OpenIRA* ir_obj = new OpenIRA();
    ir_obj->import_lef(parmsToOpenira->getInputLefFile().c_str()); 
    ir_obj->import_def(parmsToOpenira->getInputDefFile().c_str()); ;
    ir_obj->import_verilog(parmsToOpenira->getInputVerilogFile().c_str());
    ir_obj->set_top_module(parmsToOpenira->getTopModule().c_str());
    ir_obj->import_lib(parmsToOpenira->getInputLibFile().c_str());
    ir_obj->import_sdc(parmsToOpenira->getInputSDCFile().c_str());
    ir_obj->read_voltage_src(parmsToOpenira->getInputVsrcFile().c_str());
    cout <<parmsToOpenira->getInputVsrcFile().c_str()<< endl;
    //ir_obj->import_db("/home/sachin00/chhab011/PDN.db");
    IRSolver* irsolve_h = new IRSolver(ir_obj->db, ir_obj->verilog_stor, ir_obj->top_cell_name, ir_obj->sdc_file, ir_obj->lib_stor, ir_obj->vsrc_loc);
    gmat_obj = irsolve_h->GetGMat();
    irsolve_h->solve_ir();
    std::vector<Node*> nodes = gmat_obj->getNodes();    
    int unit_micron = ((ir_obj->db)->getTech())->getDbUnitsPerMicron();

    ofstream current_file;
    ofstream voltage_file;
    int vsize;
    current_file.open ("J.csv");
    voltage_file.open ("V.csv");
    vsize = nodes.size();
    for (int n=0; n<vsize; n++)
    {
        //myfile <<  std::setprecision(10)<<test_J[n] <<"\n";
        Node* node = nodes[n];
        if(node->GetLayerNum() !=1)
            continue;
        NodeLoc loc = node->GetLoc();
        current_file << double(loc.first)/unit_micron<<","<<double(loc.second)/unit_micron<<","<< std::setprecision(10)<<node->getCurrent() <<"\n";
        voltage_file << double(loc.first)/unit_micron<<","<<double(loc.second)/unit_micron<<","<< std::setprecision(10)<<node->getVoltage() <<"\n";
    }
    cout << "\n" <<endl;
    cout << "######################################" <<endl;
    cout<< "Worstcase Voltage: " << irsolve_h-> wc_voltage << endl;
    cout<< "Average  IR drop : " << irsolve_h->vdd - irsolve_h->avg_voltage << endl;
    cout<< "Worstcase IR drop: " << irsolve_h->vdd - irsolve_h->wc_voltage << endl;
    cout << "######################################" <<endl;

    current_file.close();
    voltage_file.close();


    return 1;

}


int main(int argc, char** argv) {
        std::cout << " ######################################\n";
        std::cout << " # OpenIRA: OpenROAD IR analysis tool #\n";
        std::cout << " #       University of Minnesota      #\n";
        std::cout << " # Author:                            #\n";
        std::cout << " #    Vidya Chhabria (UMN)            #\n";
        std::cout << " #    Sachin Sapatnekar (UMN)         #\n";
        std::cout << " ######################################\n";
        std::cout << "\n";

        Tcl_Interp *interp;

        Parameters* parmsToOpenira = new Parameters(argc, argv);
        
        if (parmsToOpenira->isInteractiveMode()) {
                Tcl_Main(argc, argv, OpeniraTclAppInit);
        } else {
                Openira(parmsToOpenira);
        }

        return 0;
}



