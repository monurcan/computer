enum segmentType {CONSTANT, ARGUMENT, LOCAL, STATIC, THIS, THAT, POINTER, TEMP};
std::map<segmentType, std::string> segmentType2String = {{CONSTANT, "constant"}, {ARGUMENT, "argument"}, {LOCAL, "local"}, {STATIC, "static"}, {THIS, "this"}, {THAT, "that"}, {POINTER, "pointer"}, {TEMP, "temp"}};
enum cmdType {ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT};
std::map<cmdType, std::string> cmdType2String = {{ADD, "add"}, {SUB, "sub"}, {NEG, "neg"}, {EQ, "eq"}, {GT, "gt"}, {LT, "lt"}, {AND, "and"}, {OR, "or"}, {NOT, "not"}};

class VMWriter{
private:
    std::ofstream write;
    
public:
    VMWriter(std::string fileName){
        fileName.erase(fileName.find(".jack"), std::string(".jack").length());
	    write.open((fileName+".vm").c_str());
    }

    void writePush(segmentType segment, int index){
        write << "push "<<segmentType2String[segment]<<" "<<index<<"\n";
    }

    void writePop(segmentType segment, int index){
        write << "pop "<<segmentType2String[segment]<<" "<<index<<"\n";
    }

    void writeArithmetic(cmdType cmd){
        write <<cmdType2String[cmd]<<"\n";
    }

    void writeLabel(std::string label){
        write << "label "<<label<<"\n";
    }

    void writeGoto(std::string label){
        write << "goto "<<label<<"\n";
    }

    void writeIf(std::string label){
        write << "if-goto "<<label<<"\n";
    }

    void writeCall(std::string name, int nArgs){
        write << "call "<<name<<" "<<nArgs<<"\n";
    }

    void writeFunction(std::string name, int nVars){
        write << "function "<<name<<" "<<nVars<<"\n";
    }    

    void writeReturn(){
        write << "return\n";
    }    

    ~VMWriter(){
        write.close();
    }
};