#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_

#include <string>
#include <vector>


class Parameters {
private:
        std::string _inputLefFile;
        std::string _inputDefFile;
        std::string _inputVerilogFile;
        std::string _inputSDCFile;
        std::string _inputLibFile;
        std::string _inputDBFile;
        std::string _inputTopModule;
        std::string _inputVsrcFile;
        bool _interactiveMode = false; 
        
public:
        Parameters() = default;
        Parameters(int, char**);
        
        void setInputDefFile(const std::string& file) { _inputDefFile = file; }
        std::string getInputDefFile() const { return _inputDefFile; }
        
        
        void setInputLefFile(const std::string& file) { _inputLefFile = file; }
        std::string getInputLefFile() const { return _inputLefFile; }
       

        void setInputSDCFile(const std::string& file) { _inputSDCFile = file; }
        std::string getInputSDCFile() const { return _inputSDCFile; }
        
        void setTopModule(const std::string& file) { _inputTopModule = file; }
        std::string getTopModule() const { return _inputTopModule; }
        
        
        void setInputLibFile(const std::string& file) { _inputLibFile = file; }
        std::string getInputLibFile() const { return _inputLibFile; }

        void setInputVerilogFile(const std::string& file) { _inputVerilogFile = file; }
        std::string getInputVerilogFile() const { return _inputVerilogFile; }
        
        
        void setInputVsrcFile(const std::string& file) { _inputVsrcFile = file; }
        std::string getInputVsrcFile() const { return _inputVsrcFile; }

        void setInteractiveMode(bool enable) { _interactiveMode = enable; }
        bool isInteractiveMode() const { return _interactiveMode; }
        
        void printAll() const;
};


#endif /* __PARAMETERS_H_ */
