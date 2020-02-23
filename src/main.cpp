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
#include "pdnsim_external.h"
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
//#include "parameters.h"

using namespace std;

extern "C" {
int Irsolver_Init(Tcl_Interp* interp);
}

int PDNSimTclAppInit(Tcl_Interp* interp)
{
  std::cout << " > Running PDNSim in interactive mode.\n";

  Tcl_Init(interp);
  Irsolver_Init(interp);

  return TCL_OK;
}


int main(int argc, char** argv)
{
  std::cout << " ######################################\n";
  std::cout << " # PDNSim: OpenROAD PDN analysis tool #\n";
  std::cout << " #       University of Minnesota      #\n";
  std::cout << " # Author:                            #\n";
  std::cout << " #    Vidya Chhabria (UMN)            #\n";
  std::cout << " #    Sachin Sapatnekar (UMN)         #\n";
  std::cout << " ######################################\n";
  std::cout << "\n";

  Tcl_Interp* interp;

  //Parameters* parmsToPDNSim = new Parameters(argc, argv);

  //if (parmsToPDNSim->isInteractiveMode()) {
    Tcl_Main(argc, argv, PDNSimTclAppInit);
  //} else {
//pdn_sim(parmsToPDNSim);
  //}

  return 0;
}
