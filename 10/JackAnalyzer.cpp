#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <bitset>
#include <vector>
#include <set>
#include <regex>
#include <experimental/filesystem>

#include "JackTokenizer.cpp"
#include "CompilationEngine.cpp"

int main(int argc, char const *argv[])
{
    std::vector<std::string> listOfFiles;

    if(std::string(argv[1]).find(".jack") != std::string::npos){
        listOfFiles.push_back(argv[1]);
    }else {
        for (auto& p : std::experimental::filesystem::recursive_directory_iterator(std::string(argv[1])))
        if (p.path().extension() == ".jack")
            listOfFiles.push_back(std::string(p.path()));        
    }

    std::unique_ptr<JackTokenizer> po;
    std::unique_ptr<CompilationEngine> co;

    for(auto &p : listOfFiles){
        po.reset(new JackTokenizer(p));
        co.reset(new CompilationEngine(p, po));
        
        po->advance();
        co->compileClass();
    }
    return 0;
}
