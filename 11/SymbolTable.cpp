enum kindType {STATIC, FIELD, ARG, VAR, NONE};

struct Column {
    std::string type;
    kindType kind;
    int index;
};

class SymbolTable {
private:
    std::map<std::string, Column> table;
    int indices[4] = {};

public:
    SymbolTable(){
        
    }

    void reset(){
        indices[0] = indices[1] = indices[2] = indices[3] = 0;
    }

    void define(std::string name, std::string type, kindType kind){
        table[name] = {type, kind, indices[kind]++};
    }

    int varCount(kindType kind){
        return indices[kind];
    }

    kindType kindOf(std::string name){
        return (table.find(name) != table.end()) ? table[name].kind : NONE;
    }

    std::string typeOf(std::string name){
        return table[name].type;
    }

    int indexOf(std::string name){
        return table[name].index;
    }

    ~SymbolTable(){

    }
};