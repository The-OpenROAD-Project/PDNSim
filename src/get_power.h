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
#ifndef __IRSOLVER_Power__
#define __IRSOLVER_Power__ 0

#include <fstream>
#include <tcl.h>
#include <limits>

//#include "Machine.hh"
//#include "Liberty.hh"
//#include "StringUtil.hh"
//#include "Vector.hh"
//#include "Sta.hh"
//#include "Sdc.hh"
//#include "StaMain.hh"
//#include "Stats.hh"
//#include "Report.hh"
//#include "StringUtil.hh"
//#include "PatternMatch.hh"
//#include "PortDirection.hh"
//#include "FuncExpr.hh"
//#include "Units.hh"
//#include "MinMax.hh"
//#include "Transition.hh"
//#include "TimingRole.hh"
//#include "TimingArc.hh"
//#include "InternalPower.hh"
//#include "LeakagePower.hh"
//#include "Liberty.hh"
//#include "EquivCells.hh"
//#include "MinMax.hh"
//#include "Network.hh"
//#include "Clock.hh"
//#include "PortDelay.hh"
//#include "ExceptionPath.hh"
//#include "Graph.hh"
//#include "GraphDelayCalc.hh"
//#include "Parasitics.hh"
//#include "Wireload.hh"
//#include "DelayCalc.hh"
//#include "DcalcAnalysisPt.hh"
//#include "Corner.hh"
//#include "Tag.hh"
//#include "PathVertex.hh"
//#include "PathRef.hh"
//#include "PathEnd.hh"
//#include "PathGroup.hh"
//#include "Power.hh"
//#include "CheckTiming.hh"
//#include "CheckMinPulseWidths.hh"
//#include "Levelize.hh"
//#include "Bfs.hh"
//#include "Search.hh"
//#include "SearchPred.hh"
//#include "PathAnalysisPt.hh"
//#include "DisallowCopyAssign.hh"
//#include "Debug.hh"
//#include "Stats.hh"
//#include "PortDirection.hh"
//#include "Liberty.hh"
//#include "Network.hh"
//#include "VerilogNamespace.hh"
//#include "CheckMinPulseWidths.hh"
//#include "CheckMinPeriods.hh"
//#include "CheckMaxSkews.hh"
//#include "Search.hh"
//#include "PathExpanded.hh"
//#include "Latches.hh"
//#include "Corner.hh"
//#include "ReportPath.hh"
//#include "VisitPathGroupVertices.hh"
//#include "WorstSlack.hh"
//#include "ParasiticsClass.hh"
//#include "ParseBus.hh"

#include "Machine.hh"
#include "Report.hh"
#include "Debug.hh"
#include "PortDirection.hh"
#include "TimingRole.hh"
#include "Units.hh"
#include "Liberty.hh"
#include "TimingArc.hh"
#include "TimingModel.hh"
#include "Corner.hh"
#include "DcalcAnalysisPt.hh"
#include "Graph.hh"
#include "ArcDelayCalc.hh"
#include "GraphDelayCalc.hh"
#include "Parasitics.hh"
#include "Sdc.hh"
#include "PathVertex.hh"
#include "SearchPred.hh"
#include "Bfs.hh"
#include "Search.hh"
#include "Network.hh"
#include "InternalPower.hh"
#include "LeakagePower.hh"
#include "Power.hh"
#include "StaMain.hh"
#include "db_sta/dbSta.hh"
#include "db_sta/dbNetwork.hh"

// to import TCL functions
//namespace sta {
//class dbSta;
//}  // namespace sta


//!  Calculates the power per instance using OpenSTA 
/*!
  Uses OpenSTA to report total power per instance and 
  use it for IR drop estimation.
*/
class PowerInst
{
 private:
  //! Instance to OpenSTA object.
  sta::dbSta*   _sta;

 public:
  //! Function for power per instance calculation
    std::vector<std::pair<std::string, double>> executePowerPerInst(
      sta::dbSta* sta);
};
 // epower namespace end
#endif
