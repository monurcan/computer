#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <bitset>

enum cmdtype {C_ARITHMETIC, C_PUSH, C_POP, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL};

class Parser {
private:
    std::ifstream read;
    std::string line;
    std::string cmd;

public:
    Parser(std::string fileName){
	    read.open((fileName+".vm").c_str());
    }

    bool advance(){
        while(std::getline(read, line)){
			line = line.substr(0, line.find("//"));
            if(!line.empty() && *line.rbegin() == '\r') {
                line.erase( line.length()-1, 1);
            }
		    if(line.find_first_not_of(' ') != std::string::npos){
                std::cout << line << "\n";
                for (int i=0; i<line.length(); i++)
                    std::cout << (int)line[i];
                std::cout << "\n\n\n";
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
    }

    std::string arg1(){
        if(commandType()==C_ARITHMETIC) return cmd;
        else if(commandType()==C_PUSH || commandType()==C_POP){
            return line.substr(line.find(" ")+1, line.find_last_of(" ")-line.find(" ")-1);
        }
    }

    int arg2(){
        return std::stoi(line.substr(line.find_last_of(" ")));
    }
};

class CodeWriter {
private:
    std::ofstream write;
    std::map<std::string, std::string> segmentSign = {{"local", "LCL"},{"argument","ARG"},{"this","THIS"},{"that","THAT"},{"temp","TEMP"}};
    std::string fileName;

public:
    CodeWriter(std::string fileName_){
	    write.open((fileName_+".asm").c_str());
        if(fileName_.find("/") != std::string::npos)
            fileName = fileName_.substr(fileName_.find_last_of("/")+1);
        else
            fileName = fileName_;
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
                        "M=D";
            }
        }
    }

    ~CodeWriter(){
        write << "//ENDOFPROGRAM\n(EOP)\n@EOP\n0;JMP";
        write.close();
    }
};

int main(int argc, char const *argv[])
{
	Parser po(argv[1]);
    CodeWriter co(argv[1]);

    while(po.advance()){
        switch (po.commandType())
        {
        case C_PUSH:
        case C_POP:
            co.writePushPop(po.commandType(), po.arg1(), po.arg2());
            break;

        case C_ARITHMETIC:
            co.writeArithmetic(po.arg1());
            break;
        
        default:
            break;
        }
    }
    return 0;
}
