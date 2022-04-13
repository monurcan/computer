#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <bitset>
#include <vector>
#include <experimental/filesystem>

enum cmdtype {C_ARITHMETIC, C_PUSH, C_POP, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL};

class Parser {
private:
    std::ifstream read;
    std::string line;
    std::string cmd;

public:
    Parser(std::string fileName){
	    read.open(fileName.c_str());
    }

    bool advance(){
        while(std::getline(read, line)){
            line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );
            line.erase( std::remove(line.begin(), line.end(), '\t'), line.end() );
		    if(line.find("//") != std::string::npos) line.erase(line.find("//"));
            if(line.find_last_not_of(' ') != std::string::npos) line.erase(line.find_last_not_of(' ')+1);
		    
            if(line.find_first_not_of(' ') != std::string::npos){
                line.erase(0, line.find_first_not_of(' '));
                //std::cout << line << "| " << std::endl;
                return true;
            }
		}
		return false;
    }

    cmdtype commandType(){
		cmd = line.substr(0, line.find(" "));
        if(cmd=="push") return C_PUSH;
        else if(cmd=="pop") return C_POP;
        else if(cmd=="add"||cmd=="sub"||cmd=="neg"||cmd=="eq"||cmd=="gt"||cmd=="lt"||cmd=="and"||cmd=="or"||cmd=="not") return C_ARITHMETIC;
        else if(cmd=="label") return C_LABEL;
        else if(cmd=="if-goto") return C_IF;
        else if(cmd=="goto") return C_GOTO;
        else if(cmd=="function") return C_FUNCTION;
        else if(cmd=="return") return C_RETURN;
        else if(cmd=="call") return C_CALL;
    }

    std::string arg1(){
        if(commandType()==C_ARITHMETIC) return cmd;
        else return line.substr(line.find(" ")+1, line.find_last_of(" ")-line.find(" ")-1);
    }

    int arg2(){
        return std::stoi(line.substr(line.find_last_of(" ")));
    }

    ~Parser(){
        read.close();
    }

};

class CodeWriter {
private:
    std::ofstream write;
    std::map<std::string, std::string> segmentSign = {{"local", "LCL"},{"argument","ARG"},{"this","THIS"},{"that","THAT"},{"temp","TEMP"}};
    std::string fileName;
    std::string currentFunctionPostfix;
    int callCounter = 0;

public:
    CodeWriter(std::string fileName__){
        std::string fileName_ = fileName__;
        if(fileName_.back() == '/')
            fileName_ = fileName_.substr(0, fileName_.find_last_of("/"));
        if(fileName_.find(".vm") != std::string::npos)
            fileName_ = fileName_.erase(fileName_.find(".vm"), std::string(".vm").length());
        else
            fileName_ = fileName_+"/"+fileName_.substr(fileName_.find_last_of("/")+1);

	    write.open((fileName_+".asm").c_str());

        write << "//Bootstrap\n"
                    "@256\n"
                    "D=A\n"
                    "@SP\n"
                    "M=D\n";
        currentFunctionPostfix = "Bootstrap";
        writeCall("Sys.init", 0);
    }

    void setFileName(std::string fileName__){
        std::string fileName_ = fileName__.erase(fileName__.find(".vm"), std::string(".vm").length());
        if(fileName_.find("/") != std::string::npos)
            fileName = fileName_.substr(fileName_.find_last_of("/")+1);
        else
            fileName = fileName_;
        currentFunctionPostfix = fileName;
    }

    void writeArithmetic(std::string command){
        static int i = 0;
        write << std::string("//") + command + "\n";

        if(command == "add"||command=="sub"||command=="eq"||command=="gt"||command=="lt"||command=="and"||command=="or"){
            write << "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "A=A-1\n";
            if(command=="add") write << "M=D+M\n";
            else if(command=="sub") write << "M=M-D\n";
            else if(command=="eq"||command=="gt"||command=="lt"){
                write <<    "D=M-D\n"
                            "M=-1\n"
                            "@TRUE"+std::to_string(i)+"\n";
                
                if(command=="eq") write << "D;JEQ\n";
                else if(command=="gt") write << "D;JGT\n";
                else if(command=="lt") write << "D;JLT\n";

                write <<    "@SP\n"
                            "A=M-1\n"
                            "M=0\n"
                            "(TRUE"+std::to_string(i++)+")\n";
            }else if(command=="or") write << "M=D|M\n";
            else if(command=="and") write << "M=D&M\n";
        }else if(command=="neg" || command=="not"){
            write <<    "@SP\n"
                        "A=M-1\n";
            write << ((command=="neg") ? "M=-M\n" : "M=!M\n");
        }
    }

    void writePushPop(cmdtype command, std::string segment, int index){
        write << std::string("//") + (command == C_PUSH ? "push" : "pop") + " " + segment + " " + std::to_string(index) + "\n";

        if(segment == "local" || segment == "argument" || segment == "this" || segment == "that"){
            if(command == C_PUSH){
                write <<"@"+std::to_string(index)+"\n"
                        "D=A\n"
                        "@"+segmentSign[segment]+"\n"
                        "A=M+D\n"
                        "D=M\n"
                        "@SP\n"
                        "A=M\n"
                        "M=D\n"
                        "@SP\n"
                        "M=M+1\n";
            }else {
                write <<"@"+std::to_string(index)+"\n"
                        "D=A\n"
                        "@"+segmentSign[segment]+"\n"
                        "D=M+D\n"
                        "@R13\n"
                        "M=D\n"
                        "@SP\n"
                        "AM=M-1\n"
                        "D=M\n"
                        "@R13\n"
                        "A=M\n"
                        "M=D\n";
            }
        }else if(segment == "constant"){
            write <<    "@"+std::to_string(index)+"\n"
                        "D=A\n"
                        "@SP\n"
                        "A=M\n"
                        "M=D\n"
                        "@SP\n"
                        "M=M+1\n";
        }else if(segment == "temp"){
            if(command == C_PUSH){
                write <<"@"+std::to_string(5+index)+"\n"
                        "D=M\n"
                        "@SP\n"
                        "A=M\n"
                        "M=D\n"
                        "@SP\n"
                        "M=M+1\n";
            }else {
                write <<"@SP\n"
                        "AM=M-1\n"
                        "D=M\n"
                        "@"+std::to_string(5+index)+"\n"
                        "M=D\n";
            }
        }else if(segment == "static"){
            if(command == C_PUSH){
                write <<"@"+ fileName + "." + std::to_string(index)+"\n"
                        "D=M\n"
                        "@SP\n"
                        "A=M\n"
                        "M=D\n"
                        "@SP\n"
                        "M=M+1\n";
            }else {
                write <<"@SP\n"
                        "AM=M-1\n"
                        "D=M\n"
                        "@"+ fileName + "." + std::to_string(index)+"\n"
                        "M=D\n";
            }
        }else if(segment == "pointer"){
            if(command == C_PUSH){
                write <<std::string("@")+(index==0 ? "THIS" : "THAT")+"\n"
                        "D=M\n"
                        "@SP\n"
                        "A=M\n"
                        "M=D\n"
                        "@SP\n"
                        "M=M+1\n";
            }else {
                write <<std::string("@SP\n"
                        "AM=M-1\n"
                        "D=M\n"
                        "@")+(index==0 ? "THIS" : "THAT")+"\n"
                        "M=D\n";
            }
        }
    }

    void writeLabel(std::string label){
        write <<    "//label "+label+"\n"
                    "("+currentFunctionPostfix+"$"+label+")\n";
    }

    void writeGoto(std::string label){
        write <<    "//goto "+label+"\n"
                    "@"+currentFunctionPostfix+"$"+label+"\n"
                    "0;JMP\n";
    }

    void writeIf(std::string label){
        write <<    "//if-goto "+label+"\n"
                    "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "@"+currentFunctionPostfix+"$"+label+"\n"
                    "D;JNE\n";
    }

    void writeFunction(std::string functionName, int nVars){
        write <<    "//function "+functionName+" "+std::to_string(nVars)+"\n"
                    "("+functionName+")\n";
        for(int i=0;i<nVars;i++) write << "@SP\nA=M\nM=0\n@SP\nM=M+1\n";
        
        currentFunctionPostfix = functionName;
    }

    void writeCall(std::string functionName, int nArgs){
        std::string returnAddress = currentFunctionPostfix+"$ret."+std::to_string(callCounter++);
        write <<    "//call "+functionName+" "+std::to_string(nArgs)+"\n"
                    "@"+returnAddress+"\n"
                    "D=A\n"
                    "@SP\n"
                    "A=M\n"
                    "M=D\n"

                    "@LCL\n"
                    "D=M\n"
                    "@SP\n"
                    "AM=M+1\n"
                    "M=D\n"

                    "@ARG\n"
                    "D=M\n"
                    "@SP\n"
                    "AM=M+1\n"
                    "M=D\n"

                    "@THIS\n"
                    "D=M\n"
                    "@SP\n"
                    "AM=M+1\n"
                    "M=D\n"

                    "@THAT\n"
                    "D=M\n"
                    "@SP\n"
                    "AM=M+1\n"
                    "M=D\n"

                    "@SP\n"
                    //"DM=M+1\n"
                    "MD=M+1\n"
                    "@"+std::to_string(5+nArgs)+"\n"
                    "D=D-A\n"
                    "@ARG\n"
                    "M=D\n"

                    "@SP\n"
                    "D=M\n"
                    "@LCL\n"
                    "M=D\n"

                    "@"+functionName+"\n"
                    "0;JMP\n"
                   
                    "("+returnAddress+")\n";
    }

    void writeReturn(){
        write <<    "//return\n"
                    "@LCL\n"
                    "D=M\n"
                    "@R13\n"
                    "M=D\n"
                    "@5\n"
                    "A=D-A\n"
                    "D=M\n"
                    "@R14\n"
                    "M=D\n"
                    
                    "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"

                    "@ARG\n"
                    "A=M\n"
                    "M=D\n"

                    "@ARG\n"
                    "D=M+1\n"
                    "@SP\n"
                    "M=D\n"

                    "@R13\n"
                    "A=M-1\n"
                    "D=M\n"
                    "@THAT\n"
                    "M=D\n"

                    "@2\n"
                    "D=A\n"
                    "@R13\n"
                    "A=M-D\n"
                    "D=M\n"
                    "@THIS\n"
                    "M=D\n"

                    "@3\n"
                    "D=A\n"
                    "@R13\n"
                    "A=M-D\n"
                    "D=M\n"
                    "@ARG\n"
                    "M=D\n"

                    "@4\n"
                    "D=A\n"
                    "@R13\n"
                    "A=M-D\n"
                    "D=M\n"
                    "@LCL\n"
                    "M=D\n"
                    
                    "@R14\n"
                    "A=M\n"
                    "0;JMP\n";
    }

    ~CodeWriter(){
        write << "//ENDOFPROGRAM\n(EOP)\n@EOP\n0;JMP";
        write.close();
    }
};

int main(int argc, char const *argv[])
{
    std::vector<std::string> listOfFiles;

    if(std::string(argv[1]).find(".vm") != std::string::npos){
        listOfFiles.push_back(argv[1]);
    }else {
        for (auto& p : std::experimental::filesystem::recursive_directory_iterator(std::string(argv[1])))
        if (p.path().extension() == ".vm")
            listOfFiles.push_back(std::string(p.path()));        
    }

    Parser* po = NULL;
    CodeWriter co(argv[1]);

    for(auto &p : listOfFiles){
    po = new Parser(p);
    co.setFileName(p);
    
    while(po->advance()){
        switch (po->commandType())
        {
        case C_PUSH:
        case C_POP:
            co.writePushPop(po->commandType(), po->arg1(), po->arg2());
            break;

        case C_ARITHMETIC:
            co.writeArithmetic(po->arg1());
            break;
        
        case C_LABEL:
            co.writeLabel(po->arg1());
            break;
        
        case C_GOTO:
            co.writeGoto(po->arg1());
            break;
        
        case C_IF:
            co.writeIf(po->arg1());
            break;

        case C_FUNCTION:
            co.writeFunction(po->arg1(), po->arg2());
            break;
        
        case C_RETURN:
            co.writeReturn();
            break;
        
        case C_CALL:
            co.writeCall(po->arg1(), po->arg2());
            break;

        default:
            break;
        }
    }

    }
    return 0;
}
