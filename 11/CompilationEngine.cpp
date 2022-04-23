class CompilationEngine {
private:
    std::unique_ptr<JackTokenizer> &tokenizer;
    SymbolTable::SymbolTable classLevelST, subroutineLevelST;
    VMWriter::VMWriter vmwriter;
    std::string className;
    std::map<std::string, VMWriter::cmdType> stringTocmdType = {{"+", VMWriter::ADD}, {"-", VMWriter::SUB}, {"=", VMWriter::EQ}, {">",VMWriter::GT}, {"<",VMWriter::LT}, {"&",VMWriter::AND}, {"|",VMWriter::OR}};
    static int labelCounter;
    void process(std::string str0="", std::string str1="", std::string str2="", std::string str3="", tokenType_ inp=IDENTIFIER, bool useTokenType=false){
        if((tokenizer->token == str0)||(tokenizer->token == str1)||(tokenizer->token == str2)||(tokenizer->token == str3)||((tokenizer->tokenType() == inp) && useTokenType)){
            //write << tokenizer->XMLFormattedToken() << "\n";
        }else
            std::cout << "syntax error\n";
        
        tokenizer->advance();
    }

    void process(tokenType_ inp){
        if(tokenizer->tokenType()==inp){
            //write << tokenizer->XMLFormattedToken() << "\n";
        }else
            std::cout << "syntax error\n";
        
        tokenizer->advance();
    }

public:
    CompilationEngine(std::string fileName, std::unique_ptr<JackTokenizer> &tokenizerPointer)
        : tokenizer(tokenizerPointer), vmwriter(fileName) {}    

    ~CompilationEngine(){
    }

    void compileClass(){
        //write << "<class>\n";
        process("class");
        className = tokenizer->token;
        process(IDENTIFIER);
        process("{");

        bool classVarDecsContinue = true;
        while(classVarDecsContinue){
            switch(tokenizer->keyWord()){
                case STATIC: case FIELD: compileClassVarDec(); break;
                default: classVarDecsContinue = false; break;
            }
        }
        
        bool classSubroutinesContinue = true;
        while(classSubroutinesContinue){
            switch(tokenizer->keyWord()){
                case CONSTRUCTOR: case FUNCTION: case METHOD: compileSubroutine(); break;
                default: classSubroutinesContinue = false; break;
            }
        }

        process("}");
        //write << "</class>\n";
    }

    void compileClassVarDec(){
        //write << "<classVarDec>\n";
        SymbolTable::kindType kind = (tokenizer->token=="static")?SymbolTable::STATIC:SymbolTable::FIELD;
        process("static", "field");
        std::string type = tokenizer->token;
        process("int", "char", "boolean", "", IDENTIFIER, true);
        std::string name = tokenizer->token;
        process(IDENTIFIER);
        classLevelST.define(name, type, kind);
        
        bool varNamesContinue = true;
        while(varNamesContinue){
            if(tokenizer->token == ","){
                process(",");
                std::string name = tokenizer->token;
                process(IDENTIFIER);
                classLevelST.define(name, type, kind);
            }else varNamesContinue = false;
        }

        process(";");
        //write << "</classVarDec>\n";
    }

    void compileSubroutine(){
        //write << "<subroutineDec>\n";
        subroutineLevelST.reset();
        if(tokenizer->token=="method") subroutineLevelST.define("this", className, SymbolTable::ARG);
        std::string typeFunc = tokenizer->token;
        process("constructor", "function", "method");
        process("void", "int", "char", "boolean", IDENTIFIER, true);
        std::string funcName = tokenizer->token;
        process(IDENTIFIER);
        process("(");
        compileParameterList();
        process(")");
        compileSubroutineBody(funcName, typeFunc);
        //write << "</subroutineDec>\n";
    }

    void compileParameterList(){
        //write << "<parameterList>\n";
        if(tokenizer->token=="int"||tokenizer->token=="char"||tokenizer->token=="boolean"||tokenizer->tokenType()==IDENTIFIER){
        std::string type = tokenizer->token;
        process("int", "char", "boolean", "", IDENTIFIER, true);
        std::string name = tokenizer->token;
        process(IDENTIFIER);
        subroutineLevelST.define(name, type, SymbolTable::ARG);

        bool parameterListContinue = true;
        while(parameterListContinue){
            if(tokenizer->token == ","){
                process(",");
                std::string type = tokenizer->token;
                process("int", "char", "boolean", "", IDENTIFIER, true);
                std::string name = tokenizer->token;
                process(IDENTIFIER);
                subroutineLevelST.define(name, type, SymbolTable::ARG);
            }else parameterListContinue = false;
        }
        }
        //write << "</parameterList>\n";
    }

    void compileSubroutineBody(std::string funcName, std::string typeFunc){
        //write << "<subroutineBody>\n";
        process("{");
        
        int varCount = 0;
        bool varDecContinue = true;
        while(varDecContinue){
            if(tokenizer->token == "var"){ compileVarDec(varCount); varCount++;}
            else varDecContinue = false;
        }
        
        vmwriter.writeFunction(className+"."+funcName, varCount);
        if(typeFunc == "method"){
            vmwriter.writePush(VMWriter::ARGUMENT, 0);
            vmwriter.writePop(VMWriter::POINTER, 0);
        }else if(typeFunc == "constructor"){
            vmwriter.writePush(VMWriter::CONSTANT, classLevelST.varCount(SymbolTable::FIELD));
            vmwriter.writeCall("Memory.alloc", 1);
            vmwriter.writePop(VMWriter::POINTER, 0);
        }
        compileStatements();
        process("}");
        //write << "</subroutineBody>\n";
    }

    void compileVarDec(int &varCount){
        //write << "<varDec>\n";
        process("var");
        std::string type = tokenizer->token;
        process("int", "char", "boolean", "", IDENTIFIER, true);
        std::string name = tokenizer->token;
        process(IDENTIFIER);
        subroutineLevelST.define(name, type, SymbolTable::VAR);
        
        bool varNamesContinue = true;
        while(varNamesContinue){
            if(tokenizer->token == ","){
                process(",");
                std::string name = tokenizer->token;
                process(IDENTIFIER);
                subroutineLevelST.define(name, type, SymbolTable::VAR);
                varCount++;
            }else varNamesContinue = false;
        }

        process(";");
        //write << "</varDec>\n";
    }

    void compileStatements(){
        //write << "<statements>\n";
        bool statementsContinue = true;
        while(statementsContinue){
            switch(tokenizer->keyWord()){
                case LET: compileLet(); break;
                case IF: compileIf(); break;
                case WHILE: compileWhile(); break;
                case DO: compileDo(); break;
                case RETURN: compileReturn(); break;
                default: statementsContinue = false; break;
            }
        }
        //write << "</statements>\n";
    }

    void compileLet(){
        //write << "<letStatement>\n";
        process("let");
        bool isLeftArray = false;
        std::string varName = tokenizer->token;
        process(IDENTIFIER);

        if(tokenizer->token == "["){
            isLeftArray = true;

            if(subroutineLevelST.kindOf(varName) != SymbolTable::NONE)
                vmwriter.writePush(subroutineLevelST.kindOf(varName) == SymbolTable::VAR ? VMWriter::LOCAL : VMWriter::ARGUMENT, subroutineLevelST.indexOf(varName));
            else if(classLevelST.kindOf(varName) != SymbolTable::NONE)
                vmwriter.writePush(classLevelST.kindOf(varName) == SymbolTable::FIELD ? VMWriter::THIS : VMWriter::STATIC, classLevelST.indexOf(varName));

            process("[");
            compileExpression();
            process("]");

            vmwriter.writeArithmetic(VMWriter::ADD);
        }

        process("=");
        compileExpression();
        process(";");
        
        if(isLeftArray){
            vmwriter.writePop(VMWriter::TEMP, 0);
            vmwriter.writePop(VMWriter::POINTER, 1);
            vmwriter.writePush(VMWriter::TEMP, 0);
            vmwriter.writePop(VMWriter::THAT, 0);
        }else {
            if(subroutineLevelST.kindOf(varName) != SymbolTable::NONE)
                vmwriter.writePop(subroutineLevelST.kindOf(varName) == SymbolTable::VAR ? VMWriter::LOCAL : VMWriter::ARGUMENT, subroutineLevelST.indexOf(varName));
            else if(classLevelST.kindOf(varName) != SymbolTable::NONE)
                vmwriter.writePop(classLevelST.kindOf(varName) == SymbolTable::FIELD ? VMWriter::THIS : VMWriter::STATIC, classLevelST.indexOf(varName));
        }
        //write << "</letStatement>\n";

    }

    void compileIf(){
        //write << "<ifStatement>\n";
        int labelNumber = labelCounter;
        labelCounter += 2;

        process("if");
        process("(");
        compileExpression();
        process(")");
        process("{");
        vmwriter.writeArithmetic(VMWriter::NOT);
        vmwriter.writeIf("L"+std::to_string(labelNumber));
        compileStatements();
        vmwriter.writeGoto("L"+std::to_string(labelNumber+1));
        vmwriter.writeLabel("L"+std::to_string(labelNumber));
        process("}");
        
        if(tokenizer->keyWord() == ELSE){
            process("else");
            process("{");
            compileStatements();
            process("}");
        }
        vmwriter.writeLabel("L"+std::to_string(labelNumber+1));

        //write << "</ifStatement>\n"; 
    }

    void compileWhile(){
        //write << "<whileStatement>\n";
        int labelNumber = labelCounter;
        labelCounter += 2;
        process("while");
        process("(");
        vmwriter.writeLabel("L"+std::to_string(labelNumber));
        compileExpression();
        vmwriter.writeArithmetic(VMWriter::NOT);
        process(")");
        process("{");
        vmwriter.writeIf("L"+std::to_string(labelNumber+1));
        compileStatements();
        vmwriter.writeGoto("L"+std::to_string(labelNumber));
        vmwriter.writeLabel("L"+std::to_string(labelNumber+1));
        process("}");
        //write << "</whileStatement>\n";
    }

    void compileDo(){
        //write << "<doStatement>\n";
        process("do");

        compileExpression(true);
        vmwriter.writePop(VMWriter::TEMP, 0);
        process(";");
        //write << "</doStatement>\n";
    }

    void compileReturn(){
        //write << "<returnStatement>\n";
        process("return");
        if(tokenizer->token != ";") compileExpression();
        else vmwriter.writePush(VMWriter::CONSTANT, 0);
        vmwriter.writeReturn();
        process(";");
        //write << "</returnStatement>\n";
    }

    void compileExpression(bool cameFromDo=false){
        if(!cameFromDo){} //write << "<expression>\n";
        compileTerm(cameFromDo);

        bool termsContinue = true;
        while(termsContinue){
            std::string stringcmd = tokenizer->token;
            if(tokenizer->token == "+"||tokenizer->token == "-"||tokenizer->token == "*"||tokenizer->token == "/"){
                process("+","-","*","/");
            }else if(tokenizer->token == "&"||tokenizer->token == "|"||tokenizer->token == "<"||tokenizer->token == ">"){
                process("&","|","<",">");
            }else if(tokenizer->token == "="){
                process("=");
            }else{termsContinue = false; break;}
            compileTerm();
            if(stringcmd == "*"){
                vmwriter.writeCall("Math.multiply", 2);
            }else if(stringcmd == "/"){
                vmwriter.writeCall("Math.divide", 2);
            }else {
                vmwriter.writeArithmetic(stringTocmdType[stringcmd]);
            }
        }
        if(!cameFromDo){} //write << "</expression>\n";
    }

    void compileTerm(bool cameFromDo=false){
        if(!cameFromDo){} //write << "<term>\n";
        
        //std::cout<<"\n***||"<<tokenizer->token<<"||***\n";
        
        if(tokenizer->token == "("){
            process("(");
            compileExpression();
            process(")");
        }else if(tokenizer->token=="-"||tokenizer->token=="~"){
            VMWriter::cmdType prefixToken = tokenizer->token == "-" ? VMWriter::NEG : VMWriter::NOT;
            process("-","~");
            compileTerm();
            vmwriter.writeArithmetic(prefixToken);
        }else if(tokenizer->token=="true"||tokenizer->token=="false"||tokenizer->token=="null"||tokenizer->token=="this"||tokenizer->tokenType()==STRING_CONST||tokenizer->tokenType()==INT_CONST){
            if(tokenizer->token=="false"||tokenizer->token=="null"){
                vmwriter.writePush(VMWriter::CONSTANT, 0);
            }else if(tokenizer->token=="true"){
                vmwriter.writePush(VMWriter::CONSTANT, 1);
                vmwriter.writeArithmetic(VMWriter::NEG);
            }else if(tokenizer->token=="this"){
                vmwriter.writePush(VMWriter::POINTER, 0);
            }else if(tokenizer->tokenType()==STRING_CONST){
                vmwriter.writePush(VMWriter::CONSTANT, (tokenizer->stringVal()).length());
                vmwriter.writeCall("String.new", 1);
                for(char c : tokenizer->stringVal()){
                    vmwriter.writePush(VMWriter::CONSTANT, int(c));
                    vmwriter.writeCall("String.appendChar", 2);
                }
            }else if(tokenizer->tokenType()==INT_CONST){
                    vmwriter.writePush(VMWriter::CONSTANT, tokenizer->intVal());
            }
            process(tokenizer->token);
        }else if(tokenizer->tokenType()==IDENTIFIER){
            std::string varNameOrSubroutineCall = tokenizer->token;
            process(IDENTIFIER);

            if(tokenizer->token == "["){
                if(subroutineLevelST.kindOf(varNameOrSubroutineCall) != SymbolTable::NONE)
                    vmwriter.writePush(subroutineLevelST.kindOf(varNameOrSubroutineCall) == SymbolTable::VAR ? VMWriter::LOCAL : VMWriter::ARGUMENT, subroutineLevelST.indexOf(varNameOrSubroutineCall));
                else if(classLevelST.kindOf(varNameOrSubroutineCall) != SymbolTable::NONE)
                    vmwriter.writePush(classLevelST.kindOf(varNameOrSubroutineCall) == SymbolTable::FIELD ? VMWriter::THIS : VMWriter::STATIC, classLevelST.indexOf(varNameOrSubroutineCall));
                process("[");
                compileExpression();
                process("]");
                vmwriter.writeArithmetic(VMWriter::ADD);
                vmwriter.writePop(VMWriter::POINTER, 1);
                vmwriter.writePush(VMWriter::THAT, 0);
            }else if(tokenizer->token == "." || tokenizer->token == "("){
                bool isMethod = true;
                if(tokenizer->token == "."){
                    if(subroutineLevelST.kindOf(varNameOrSubroutineCall) != SymbolTable::NONE){
                        vmwriter.writePush(subroutineLevelST.kindOf(varNameOrSubroutineCall) == SymbolTable::VAR ? VMWriter::LOCAL : VMWriter::ARGUMENT, subroutineLevelST.indexOf(varNameOrSubroutineCall));
                        varNameOrSubroutineCall = subroutineLevelST.typeOf(varNameOrSubroutineCall);
                    }else if(classLevelST.kindOf(varNameOrSubroutineCall) != SymbolTable::NONE){
                        vmwriter.writePush(classLevelST.kindOf(varNameOrSubroutineCall) == SymbolTable::FIELD ? VMWriter::THIS : VMWriter::STATIC, classLevelST.indexOf(varNameOrSubroutineCall));
                        varNameOrSubroutineCall = classLevelST.typeOf(varNameOrSubroutineCall);
                    }else
                        isMethod = false;
                    process(".");
                    varNameOrSubroutineCall += "."+tokenizer->token;
                    process(IDENTIFIER);
                }else {
                    vmwriter.writePush(VMWriter::POINTER, 0);
                    varNameOrSubroutineCall = className + "." + varNameOrSubroutineCall;
                }

                process("(");
                int numberofArgs = compileExpressionList();
                if(isMethod){numberofArgs++;}
                process(")");
                vmwriter.writeCall(varNameOrSubroutineCall, numberofArgs);
            }else {
                if(subroutineLevelST.kindOf(varNameOrSubroutineCall) != SymbolTable::NONE)
                    vmwriter.writePush(subroutineLevelST.kindOf(varNameOrSubroutineCall) == SymbolTable::VAR ? VMWriter::LOCAL : VMWriter::ARGUMENT, subroutineLevelST.indexOf(varNameOrSubroutineCall));
                else if(classLevelST.kindOf(varNameOrSubroutineCall) != SymbolTable::NONE)
                    vmwriter.writePush(classLevelST.kindOf(varNameOrSubroutineCall) == SymbolTable::FIELD ? VMWriter::THIS : VMWriter::STATIC, classLevelST.indexOf(varNameOrSubroutineCall));
            }
        }else {
            std::cout << "-syntax error\n";
        }
        
        if(!cameFromDo){} //write << "</term>\n";}
    }

    int compileExpressionList(){
        //write << "<expressionList>\n";
        //TERM HEAD
        int expressionCount = 0;
        if(tokenizer->tokenType()==IDENTIFIER||tokenizer->tokenType()==STRING_CONST||tokenizer->tokenType()==INT_CONST||tokenizer->token=="true"||tokenizer->token=="false"||tokenizer->token=="null"||tokenizer->token=="this"||tokenizer->token=="("||tokenizer->token=="-"||tokenizer->token=="~"){
        compileExpression();
        expressionCount++;

        bool expressionListContinue = true;
        while(expressionListContinue){
            if(tokenizer->token == ","){
                process(",");
                compileExpression();
                expressionCount++;
            }else expressionListContinue = false;
        }
        }
        //write << "</expressionList>\n";
        return expressionCount;
    }

};

int CompilationEngine::labelCounter = 0;