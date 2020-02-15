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
#include "parameters.h"

#include <iostream>
#include <iomanip>
#include <boost/program_options.hpp>

Parameters::Parameters(int argc, char** argv)
{
  namespace po = boost::program_options;
  po::options_description dscp("Usage");
  // clang-format off
        dscp.add_options()
                ("interactive,c"          , po::value<bool>()            , "Enables interactive mode")
                ("input-lef, lef"          , po::value<std::string>()     , "Input LEF file (mandatory)")
                ("input-def,def"          , po::value<std::string>()     , "Input DEF file (mandatory)")
                ("input-db,db"            , po::value<std::string>()     , "Import DB file (optional if lef and def not specified)")
                ("input-lib,lib"          , po::value<std::string>()     , "Input LIB file (mandatory)")
                ("input-verilog,verilog"  , po::value<std::string>()     , "Input Verilog file (mandatory)")
                ("input-sdc,sdc"          , po::value<std::string>()     , "Input SDC file (mandatory)")
                ("top-module,module-name" , po::value<std::string>()     , "Verilog top module name (mandatory)")
                ("input-vsrc,voltage-src"   , po::value<std::string>()     , "Voltage source file (mandatory)")
                ;
  // clang-format on

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(dscp)
                  .style(po::command_line_style::unix_style
                         | po::command_line_style::allow_long_disguise)
                  .run(),
              vm);
    po::notify(vm);

    if (vm.count("help")
        || (!vm.count("interactive")
            && (!vm.count("input-verilog") || !vm.count("input-lef")
                || !vm.count("input-def") || !vm.count("input-sdc")
                || !vm.count("input-lib")))) {
      std::cout << "\n" << dscp;
      std::exit(1);
    }

    if (vm.count("input-lef")) {
      _inputLefFile = vm["input-lef"].as<std::string>();
    }
    if (vm.count("input-def")) {
      _inputDefFile = vm["input-def"].as<std::string>();
    }
    if (vm.count("input-sdc")) {
      _inputSDCFile = vm["input-sdc"].as<std::string>();
    }
    if (vm.count("input-db")) {
      _inputDBFile = vm["input-db"].as<std::string>();
    }
    if (vm.count("input-lib")) {
      _inputLibFile = vm["input-lib"].as<std::string>();
    }
    if (vm.count("input-verilog")) {
      _inputVerilogFile = vm["input-verilog"].as<std::string>();
    }
    if (vm.count("top-module")) {
      _inputTopModule = vm["top-module"].as<std::string>();
    }
    if (vm.count("input-vsrc")) {
      _inputVsrcFile = vm["input-vsrc"].as<std::string>();
    }

    if (vm.count("interactive")) {
      _interactiveMode = vm["interactive"].as<bool>();
    }
  } catch (const po::error& ex) {
    std::cerr << ex.what() << '\n';
  }

  if (!isInteractiveMode()) {
    printAll();
  }
}

void Parameters::printAll() const
{
  // clang-format off
        std::cout << "\nOptions: \n";
        std::cout << std::setw(20) << std::left << "Input LEF file: ";
        std::cout << _inputLefFile << "\n";
        std::cout << std::setw(20) << std::left << "Input DEF file: ";
        std::cout << _inputDefFile << "\n";
        std::cout << std::setw(20) << std::left << "Input Liberty file: ";
        std::cout << _inputLibFile << "\n";
        std::cout << std::setw(20) << std::left << "Input Verilog file: ";
        std::cout << _inputVerilogFile << "\n";
        std::cout << std::setw(20) << std::left << "Input SDC file: ";
        std::cout << _inputSDCFile<< "\n";
        std::cout << std::setw(20) << std::left << "Verilog top module: ";
        std::cout << _inputTopModule<< "\n";
        std::cout << std::setw(20) << std::left << "Voltage source file: ";
        std::cout << _inputVsrcFile<< "\n";
        std::cout << std::setw(20) << std::left << "Interactive mode: ";
        std::cout << _interactiveMode << "\n";

        std::cout << "\n";
  // clang-format on
}
