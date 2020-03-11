#ifndef __PDNSim_HEADER__
#define __PDNSim_HEADER__


#include <string>

namespace odb {
  class dbDatabase;
}
namespace sta {
  class dbSta;
}

namespace pdnsim {

class PDNSim
{
  public:
    PDNSim();
    ~PDNSim();

    void init();
    void reset();

    void setDb(odb::dbDatabase* odb);
    void setSta(sta::dbSta* dbSta);
    
    void import_vsrc_cfg(std::string vsrc);
    void import_out_file(std::string out_file);

    void analyze_power_grid();

    int check_connectivity();

  private:
    odb::dbDatabase* _db;
    sta::dbSta* _sta;
    std::string _vsrc_loc;
    std::string _out_file;


};
}

#endif
