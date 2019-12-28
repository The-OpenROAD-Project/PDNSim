#ifndef __REPLACE_TIMING__
#define __REPLACE_TIMING__ 0


#include <fstream>
#include <boost/functional/hash.hpp>
#include <tcl.h>
#include <limits>

#define DBU_MAX std::numeric_limits< DBU >::max()

#define CAP_SCALE 1.0e-12
#define RES_SCALE 1.0

#define MAX_WIRE_SEGMENT_IN_MICRON 20.0
#define TIMING_PIN_CAP 4e-15

namespace sta {
class Sta;
}


namespace Timing { 

class Timing {
 private:
 
  sta::Sta* _sta;
  Tcl_Interp* _interp;

  float _targetTop;


  // For OpenSTA
  void FillSpefForSta();
  void MakeParasiticsForSta();

  void GenerateClockSta();
  void UpdateTimingSta();
  void UpdateNetWeightSta();

 public:
 // Timing(MODULE* modules, TERM* terms, NET* nets, int netCnt, PIN* pins,
 //        int pinCnt, 
 //        std::vector< std::vector< std::string > >& mPinName,
 //        std::vector< std::vector< std::string > >& tPinName, 
 //        std::string clkName, float clkPeriod);

  // Steiner point generating
  // it assumes that pin location is updated
  //
  // store wireSegment into wireSegStor
  //void BuildSteiner(bool scaleApplied = false);

  // ? not sure, but needs to be sliced?
  //        void SliceLongWire();

  //void WriteSpef(const std::string& spefFile);
  void executePowerPerInst(std::string topCellName, std::string verilogName,
                       std::vector< std::string >& libName, std::string sdcName);
  //void ExecuteStaLater();
};


}


#endif
