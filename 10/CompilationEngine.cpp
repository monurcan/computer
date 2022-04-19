class CompilationEngine {
private:
    std::unique_ptr<JackTokenizer> &tokenizer;
    std::ofstream write;

    void process(std::string str0="", std::string str1="", std::string str2="", std::string str3="", tokenType_ inp=IDENTIFIER, bool useTokenType=false){
        if((tokenizer->token == str0)||(tokenizer->token == str1)||(tokenizer->token == str2)||(tokenizer->token == str3)||((tokenizer->tokenType() == inp) && useTokenType))
            write << tokenizer->XMLFormattedToken() << "\n";
        else
            std::cout << "syntax error\n";
        
        tokenizer->advance();
    }

    void process(tokenType_ inp){
        if(tokenizer->tokenType()==inp)
            write << tokenizer->XMLFormattedToken() << "\n";
        else
            std::cout << "syntax error\n";
        
        tokenizer->advance();
    }

    void compileVarDecCommonParts(){
        process("int", "char", "boolean", "", IDENTIFIER, true);
        process(IDENTIFIER);
        
        bool varNamesContinue = true;
        while(varNamesContinue){
            if(tokenizer->token == ","){
                process(",");
                process(IDENTIFIER);
            }else varNamesContinue = false;
        }

        process(";");
    }

public:
    CompilationEngine(std::string fileName, std::unique_ptr<JackTokenizer> &tokenizerPointer)
        : tokenizer(tokenizerPointer)
    {
        fileName.erase(fileName.find(".jack"), std::string(".jack").length());
	    write.open((fileName+"_.xml").c_str());
    }    

    void compileClass(){
        write << "<class>\n";
        process("class");
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
        write << "</class>\n";
    }

    void compileClassVarDec(){
        write << "<classVarDec>\n";
        process("static", "field");
        compileVarDecCommonParts();
        write << "</classVarDec>\n";
    }

    void compileSubroutine(){
        write << "<subroutineDec>\n";
        process("constructor", "function", "method");
        process("void", "int", "char", "boolean", IDENTIFIER, true);
        process(IDENTIFIER);
        process("(");
        compileParameterList();
        process(")");
        compileSubroutineBody();
        write << "</subroutineDec>\n";
    }

    void compileParameterList(){
        write << "<parameterList>\n";
        if(tokenizer->token=="int"||tokenizer->token=="char"||tokenizer->token=="boolean"||tokenizer->tokenType()==IDENTIFIER){
        process("int", "char", "boolean", "", IDENTIFIER, true);
        process(IDENTIFIER);

        bool parameterListContinue = true;
        while(parameterListContinue){
            if(tokenizer->token == ","){
                process(",");
                process("int", "char", "boolean", "", IDENTIFIER, true);
                process(IDENTIFIER);
            }else parameterListContinue = false;
        }
        }
        write << "</parameterList>\n";
    }

    void compileSubroutineBody(){
        write << "<subroutineBody>\n";
        process("{");
        
        bool varDecContinue = true;
        while(varDecContinue){
            if(tokenizer->token == "var") compileVarDec();
            else varDecContinue = false;
        }
        
        compileStatements();
        process("}");
        write << "</subroutineBody>\n";
    }

    void compileVarDec(){
        write << "<varDec>\n";
        process("var");
        compileVarDecCommonParts();
        write << "</varDec>\n";
    }

    void compileStatements(){
        write << "<statements>\n";
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
        write << "</statements>\n";
    }

    void compileLet(){
        write << "<letStatement>\n";
        process("let");
        process(IDENTIFIER);

        if(tokenizer->token == "["){
            process("[");
            compileExpression();
            process("]");
        }

        process("=");
        compileExpression();
        process(";");
        write << "</letStatement>\n";
    }

    void compileIf(){
        write << "<ifStatement>\n";
        process("if");
        process("(");
        compileExpression();
        process(")");
        process("{");
        compileStatements();
        process("}");
        
        if(tokenizer->keyWord() == ELSE){
            process("else");
            process("{");
            compileStatements();
            process("}");
        }

        write << "</ifStatement>\n"; 
    }

    void compileWhile(){
        write << "<whileStatement>\n";
        process("while");
        process("(");
        compileExpression();
        process(")");
        process("{");
        compileStatements();
        process("}");
        write << "</whileStatement>\n";
    }

    void compileDo(){
        write << "<doStatement>\n";
        process("do");

        compileExpression(true);

        process(";");
        write << "</doStatement>\n";
    }

    void compileReturn(){
        write << "<returnStatement>\n";
        process("return");
        if(tokenizer->token != ";") compileExpression();
        process(";");
        write << "</returnStatement>\n";
    }

    void compileExpression(bool cameFromDo=false){
        if(!cameFromDo) write << "<expression>\n";
        compileTerm(cameFromDo);

        bool termsContinue = true;
        while(termsContinue){
            if(tokenizer->token == "+"||tokenizer->token == "-"||tokenizer->token == "*"||tokenizer->token == "/"){
                process("+","-","*","/");
            }else if(tokenizer->token == "&"||tokenizer->token == "|"||tokenizer->token == "<"||tokenizer->token == ">"){
                process("&","|","<",">");
            }else if(tokenizer->token == "="){
                process("=");
            }else{termsContinue = false; break;}
            compileTerm();
        }
        if(!cameFromDo) write << "</expression>\n";
    }

    void compileTerm(bool cameFromDo=false){
        if(!cameFromDo) write << "<term>\n";
        
        //std::cout<<"\n***||"<<tokenizer->token<<"||***\n";
        
        if(tokenizer->token == "("){
            process("(");
            compileExpression();
            process(")");
        }else if(tokenizer->token=="-"||tokenizer->token=="~"){
            //write << "onurdenem\n";
            process("-","~");
            compileTerm();
        }else if(tokenizer->token=="true"||tokenizer->token=="false"||tokenizer->token=="null"||tokenizer->token=="this"||tokenizer->tokenType()==STRING_CONST||tokenizer->tokenType()==INT_CONST){
            process(tokenizer->token);
        }else if(tokenizer->tokenType()==IDENTIFIER){
            process(IDENTIFIER);

            if(tokenizer->token == "["){
                process("[");
                compileExpression();
                process("]");
            }else if(tokenizer->token == "." || tokenizer->token == "("){
                if(tokenizer->token == "."){
                    process(".");
                    process(IDENTIFIER);
                }
                process("(");
                compileExpressionList();
                process(")");
            }
        }else {
            std::cout << "-syntax error\n";
        }
        
        if(!cameFromDo) write << "</term>\n";
    }

    int compileExpressionList(){
        write << "<expressionList>\n";
        //TERM HEAD
        if(tokenizer->tokenType()==IDENTIFIER||tokenizer->tokenType()==STRING_CONST||tokenizer->tokenType()==INT_CONST||tokenizer->token=="true"||tokenizer->token=="false"||tokenizer->token=="null"||tokenizer->token=="this"||tokenizer->token=="("||tokenizer->token=="-"||tokenizer->token=="~"){
        compileExpression();

        bool expressionListContinue = true;
        while(expressionListContinue){
            if(tokenizer->token == ","){
                process(",");
                compileExpression();
            }else expressionListContinue = false;
        }
        }
        write << "</expressionList>\n";
        return 0;
    }

};