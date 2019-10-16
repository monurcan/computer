#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>
#include <bitset>

enum cmdtype {A, C, L};
std::map<std::string, std::string> destMap = {{"M", "001"}, {"D", "010"}, {"MD","011"}, {"A", "100"}, {"AM", "101"}, {"AD", "110"}, {"AMD", "111"}};
std::map<std::string, std::string> compMap = {{"0", "0101010"}, {"1", "0111111"}, {"-1","0111010"}, {"D", "0001100"}, {"A", "0110000"}, {"M", "1110000"}, {"!D", "0001101"}, {"!A", "0110001"}, {"!D", "1110001"}, {"-D", "00011111"}, {"-A", "0110011"}, {"-M", "1110011"}, {"D+1", "0011111"}, {"A+1", "0110111"}, {"M+1", "1110111"}, {"D-1","0001110"}, {"A-1","0110010"}, {"M-1","1110010"}, {"D+A", "0000010"}, {"D+M", "1000010"}, {"D-A", "0010011"}, {"D-M", "1010011"}, {"A-D", "0000111"}, {"M-D", "1000111"}, {"D&A", "0000000"}, {"D&M", "1000000"}, {"D|A", "0010101"}, {"D|M", "1010101"}};
std::map<std::string, std::string> jumpMap = {{"JGT", "001"}, {"JEQ", "010"}, {"JGE", "011"}, {"JLT","100"},{"JNE", "101"},{"JLE", "110"},{"JMP", "111"}};
class Parser
{
private:
	std::string line;
	std::ifstream& read;
public:
	bool advance(){
		while(std::getline(read, line)){
			line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
			line = line.substr(0, line.find("//"));
		    if(line != "") return true;
		}
		return false;
	}

	std::string getline(){
		return line;
	}

	cmdtype getcmdtype(){
		if(line[0] == '@') return A;
		else if(line[0] == '(') return L;
		else return C;
	}

	Parser(std::ifstream &read)
	: read(read)
	{

	}
	
};

int main(int argc, char const *argv[])
{
	std::string name = "deneme";//argv[1];
	std::ifstream read;
	read.open((name+".asm").c_str());
	Parser po(read);
	std::map<std::string, int> symboltable = {{"R0", 0}, {"R1", 1}, {"R2", 2}, {"R3", 3}, {"R4", 4}, {"R5", 5}, {"R6", 6}, {"R7", 7}, {"R8", 8}, {"R9", 9}, {"R10", 10}, {"R11", 11}, {"R12", 12}, {"R13", 13}, {"R14", 14}, {"R15", 15}, {"SCREEN", 16384}, {"KBD", 24576}, {"SP", 0}, {"LCL", 1}, {"ARG", 2}, {"THIS", 3}, {"THAT", 4}};
	int currentLine = 0;

	while(po.advance()){
		if(po.getcmdtype() == L){
			symboltable[po.getline().substr(1, po.getline().length()-2)] = currentLine;
		}else {
			currentLine++;
		}
	}

	std::ofstream write;
	write.open((name+".hack").c_str());

	read.clear();
	read.seekg(0, std::ios::beg);
	int ramaddr = 16;
	while(po.advance()){
		if(po.getcmdtype() == A){
			int num = 0;
			std::istringstream iss(po.getline().substr(1));

			if (!(iss >> num).fail()) {
			    write << 0 << std::bitset<15>(num).to_string() << std::endl;
			}else {
			    if(symboltable.find(po.getline().substr(1, po.getline().length()-1)) == symboltable.end()){
			    	symboltable[po.getline().substr(1, po.getline().length()-1)] = ramaddr;
			    	write << 0 << std::bitset<15>(ramaddr++).to_string() << std::endl;
			    }else{
			    	write << 0 << std::bitset<15>(symboltable[po.getline().substr(1, po.getline().length()-1)]).to_string() << std::endl;
			    }
			}
		}else if(po.getcmdtype() == C){
			std::string dest, comp, jump, Ccmd = po.getline();
			if(Ccmd.find('=') != std::string::npos){
				dest = destMap[Ccmd.substr(0, Ccmd.find('='))];
				Ccmd.erase(0, Ccmd.find('=')+1);
			}else {
				dest = "000";
			}

			if(Ccmd.find(';') != std::string::npos){
				jump = jumpMap[Ccmd.substr(Ccmd.find(';')+1)];
				Ccmd.erase(Ccmd.find(';'));
			}else {
				jump = "000";
			}
			
			write << "111" << compMap[Ccmd] << dest << jump << std::endl;
		}
	}
	//for(std::map<std::string, int>::iterator itr = symboltable.begin(); itr != symboltable.end(); ++itr) std::cout << itr->first << '\t' << itr->second << '\n'; 
	
	return 0;
}
