enum tokenType_ {KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST};
enum keyWord_ {CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR, STATIC, FIELD, LET, DO, IF, ELSE, WHILE, RETURN, TRUE, FALSE, _NULL, THIS};

class JackTokenizer {
private:
    std::ifstream read;
    std::ofstream write;
    std::string line;
    bool commentMultiLine = false;
    static std::regex commentSignInString, wordRegex;
    static std::string keywords, symbols, integerConst, stringConst, identifierR;
    static std::map<std::string, keyWord_> keyWordMap;
    static std::map<tokenType_, std::string> tokenTypeMap;

    // Remove comments
    bool ReadLine(){
        while(std::getline(read, line)){
            if(commentMultiLine){
                if(line.find("*/") == std::string::npos){
                    continue;
                }else {
                    line.erase(0, line.find("*/")+2);
                    commentMultiLine = false;
                }
            }

            std::string line_ = regex_replace(line, commentSignInString, "");

            if(line_.find("/*") != std::string::npos){
                if(line.find("*/") != std::string::npos){
                    line.erase(line.find("/*"), line.find("*/")-line.find("/*")+2);
                }else {
                    commentMultiLine = true;
                    line.erase(line.find("/*"));
                }
            }

            line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );
            line.erase( std::remove(line.begin(), line.end(), '\t'), line.end() );
		    if(line.find("//") != std::string::npos) line.erase(line.find("//"));
            if(line.find_last_not_of(' ') != std::string::npos) line.erase(line.find_last_not_of(' ')+1);
            
            if(line.find_first_not_of(' ') != std::string::npos){
                line.erase(0, line.find_first_not_of(' '));
                return true;
            }
		}
		return false;
    }


public:
    std::string token;

    std::string XMLFormattedToken(){
        std::string result = "<" + tokenTypeMap[tokenType()] + "> ";
        if(token == "<") result+= "&lt;";
        else if(token == ">") result+= "&gt;";
        else if(token == "\"") result+= "&quot;";
        else if(token == "&") result+= "&amp;";
        else if(tokenType() == STRING_CONST) result+= stringVal();
        else result+= token;
        result += " </" + tokenTypeMap[tokenType()] + ">";
        return result;
    }

    JackTokenizer(std::string fileName){
        read.open(fileName.c_str());
        fileName.erase(fileName.find(".jack"), std::string(".jack").length());
	    write.open((fileName+"T_.xml").c_str());
        write << "<tokens>\n";
    }

    bool advance(){
        if(line.find_first_not_of(' ') == std::string::npos)
            if(!ReadLine())
                return false;

        if(line.find_last_not_of(' ') != std::string::npos) line.erase(line.find_last_not_of(' ')+1);
        if(line.find_first_not_of(' ') != std::string::npos) line.erase(0, line.find_first_not_of(' '));

        std::smatch currentWord;
        
        if(std::regex_search(line, currentWord, wordRegex)){
            token = currentWord[0].str();
            line = std::regex_replace(line, wordRegex, "", std::regex_constants::format_first_only);
            write << XMLFormattedToken() << "\n";
        }
        return true;
    }

    tokenType_ tokenType(){
        if(std::regex_match(token, std::regex(keywords))) return KEYWORD;
        else if(std::regex_match(token, std::regex(symbols))) return SYMBOL;
        else if(std::regex_match(token, std::regex(integerConst))) return INT_CONST;
        else if(std::regex_match(token, std::regex(stringConst))) return STRING_CONST;
        else if(std::regex_match(token, std::regex(identifierR))) return IDENTIFIER;
    }

    keyWord_ keyWord(){
        return keyWordMap[token];
    }

    char symbol(){
        return token.at(0);
    }

    std::string identifier(){
        return token;
    }

    int intVal(){
        return std::stoi(token);
    }

    std::string stringVal(){
        return token.substr(1, token.length()-2);
    }

    ~JackTokenizer(){
        write << "</tokens>";
        read.close();
        write.close();
    }
};

std::regex JackTokenizer::commentSignInString(R"(\"[^\"]*\/\*[^"]*\*\/[^\"]*\")"); // " /* example */ "
std::string JackTokenizer::keywords(R"(class|constructor|method|static|function|field|var|int|char|boolean|void|true|false|null|this|let|do|if|else|while|return)");
std::string JackTokenizer::symbols(R"(\{|\}|\(|\)|\[|\]|\.|,|;|\+|-|\*|\/|&|\||<|>|=|~)");
std::string JackTokenizer::integerConst(R"(\d+)");
std::string JackTokenizer::stringConst(R"("[^\"]*")");
std::string JackTokenizer::identifierR(R"([^\d\s]\w*)");
std::regex JackTokenizer::wordRegex(keywords+'|'+symbols+'|'+integerConst+'|'+stringConst+'|'+identifierR);
std::map<std::string, keyWord_> JackTokenizer::keyWordMap = {{"class", CLASS}, {"method", METHOD}, {"function", FUNCTION}, {"constructor", CONSTRUCTOR}, {"int", INT}, {"boolean", BOOLEAN}, {"char", CHAR}, {"void", VOID}, {"var",VAR}, {"static",STATIC}, {"field",FIELD}, {"let",LET}, {"do",DO}, {"if",IF}, {"else",ELSE}, {"while",WHILE}, {"return",RETURN}, {"true",TRUE}, {"false",FALSE}, {"null",_NULL}, {"this",THIS}};
std::map<tokenType_, std::string> JackTokenizer::tokenTypeMap = {{KEYWORD, "keyword"}, {SYMBOL, "symbol"}, {IDENTIFIER, "identifier"}, {INT_CONST, "integerConstant"}, {STRING_CONST, "stringConstant"}};