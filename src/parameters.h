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
#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_

#include <string>
#include <vector>

class Parameters
{
 private:
  std::string _inputLefFile;
  std::string _inputDefFile;
  std::string _inputVerilogFile;
  std::string _inputSDCFile;
  std::string _inputLibFile;
  std::string _inputDBFile;
  std::string _inputTopModule;
  std::string _inputVsrcFile;
  bool        _interactiveMode = false;

 public:
  Parameters() = default;
  Parameters(int, char**);

  void        setInputDefFile(const std::string& file) { _inputDefFile = file; }
  std::string getInputDefFile() const { return _inputDefFile; }

  void        setInputLefFile(const std::string& file) { _inputLefFile = file; }
  std::string getInputLefFile() const { return _inputLefFile; }

  void        setInputSDCFile(const std::string& file) { _inputSDCFile = file; }
  std::string getInputSDCFile() const { return _inputSDCFile; }

  void        setTopModule(const std::string& file) { _inputTopModule = file; }
  std::string getTopModule() const { return _inputTopModule; }

  void        setInputLibFile(const std::string& file) { _inputLibFile = file; }
  std::string getInputLibFile() const { return _inputLibFile; }

  void setInputVerilogFile(const std::string& file)
  {
    _inputVerilogFile = file;
  }
  std::string getInputVerilogFile() const { return _inputVerilogFile; }

  void setInputVsrcFile(const std::string& file) { _inputVsrcFile = file; }
  std::string getInputVsrcFile() const { return _inputVsrcFile; }

  void setInteractiveMode(bool enable) { _interactiveMode = enable; }
  bool isInteractiveMode() const { return _interactiveMode; }

  void printAll() const;
};

#endif /* __PARAMETERS_H_ */
