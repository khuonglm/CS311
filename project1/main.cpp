#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <string>
#include <cctype>
#include <bitset>
#include <map>

const int pcStart = 0x400000;
const int staticDataStart = 0x10000000;
const int instructionSize = 32;
const int opcodePosition = 26;
const int rsPosition = 21;
const int rtPosition = 16;
const int rdPosition = 11;
const int shtPosition = 6;

static int currentPcAddress = pcStart;
static int currentDataAddress = staticDataStart;

struct TInstruction {
	std::string mnemonic;
	char format;
	/// binary of this number includes opcode and funct
	int decimalForm;

	TInstruction(std::string m, char f = 'I', int opcode = 0, int funct = 0) {
		mnemonic = m;
		format = f;
		decimalForm = opcode << opcodePosition | funct;
	}

	bool operator < (const TInstruction& rhs) {
		return mnemonic < rhs.mnemonic;
	}
};
std::vector<TInstruction> supportInstructions = {
	TInstruction("add", 'R', 0x0, 0x20),
	TInstruction("addi", 'I', 0x8),
	TInstruction("addiu", 'I', 0x9),
	TInstruction("addu", 'R', 0x0, 0x21),
	TInstruction("and", 'R', 0x0, 0x24),
	TInstruction("andi", 'I', 0xc),
	TInstruction("beq", 'I', 0x4),
	TInstruction("bne", 'I', 0x5),
	TInstruction("j", 'J', 0x2),
	TInstruction("jal", 'J', 0x3),
	TInstruction("jr", 'R', 0x0, 0x08),
	TInstruction("lbu", 'I', 0x24),
	TInstruction("lhu", 'I', 0x25),
	TInstruction("ll", 'I', 0x30),
	TInstruction("lui", 'I', 0xf),
	TInstruction("la", 'I', 0xf), /// pseudo instruction
	TInstruction("lw", 'I', 0x23),
	TInstruction("nor", 'R', 0x0, 0x27),
	TInstruction("or", 'R', 0x0, 0x25),
	TInstruction("ori", 'I', 0xd),
	TInstruction("slt", 'R', 0x0, 0x2a),
	TInstruction("slti", 'I', 0xa),
	TInstruction("sltiu", 'I', 0xb),
	TInstruction("sltu", 'R', 0x0, 0x2b),
	TInstruction("sll", 'R', 0x0, 0x00),
	TInstruction("srl", 'R', 0x0, 0x02),
	TInstruction("sb", 'I', 0x28),
	TInstruction("sc", 'I', 0x38),
	TInstruction("sh", 'I', 0x29),
	TInstruction("sw", 'I', 0x2b),
	TInstruction("sub", 'R', 0x0, 0x22),
	TInstruction("subu", 'R', 0x0, 0x23)
};


/// mapping from data name, instruction name -> address
std::map<std::string, int> dataMapping, insMapping;
/// map to store pending address of future function (not appear yet)
/// map from label -> set of instruction that need to update when address is fixed
std::map<std::string, std::vector<std::pair<int, int>>> pendingIns;

std::string convertDecimalToBinary(int x, size_t n = 32) {
	switch (n) {
		case 5:
			return std::bitset<5>(x).to_string();
		case 6:
			return std::bitset<6>(x).to_string();
		case 16:
			return std::bitset<16>(x).to_string();
		case 26:
			return std::bitset<26>(x).to_string();
		default:
			return std::bitset<32>(x).to_string();
	}
}

/*
	convert string with format $n, 0xn, n to int
*/
int convertStringToNumber(std::string s) {
	if(s[0] == '0' && s[1] == 'x') {
		while(!((s.back() >= '0' && s.back() <= '9') || 
				('a' <= std::tolower(s.back()) && std::tolower(s.back()) <= 'f'))) 
				s.pop_back();
		return std::stoi(s, 0, 16);
	}
	while(s.back() < '0' || s.back() > '9') s.pop_back();
	if(s[0] == '$')
		return std::stoi(s.substr(1));
	return std::stoi(s);
}

int trimBits(int x, int p) {
	return x & ((1 << p) - 1);
}

void splitInputLine(std::string& s, std::vector<std::string>& ret) {
	ret.clear();
	int i = 0, j = 0, l = s.length(), open = -1, close = -1;
	while(true) {
		while(i < l && (s[i] == ' ' || s[i] == '\t')) ++i;
		j = i;
		while(j < l && !(s[j] == ' ' || s[j] == '\t')) {
			if(s[j] == '(') open = j;
			else if(s[j] == ')') close = j;
			++j;
		}
		if(i >= l) return;
		if(open != -1) {
			ret.emplace_back(s.substr(open + 1, close - open - 1));
			ret.emplace_back(s.substr(i, open - i));
			open = close = -1;
		} else {
			ret.emplace_back(s.substr(i, j - i));
		}
		i = j;
	}
}

void dataProcessing(std::vector<std::string>& data, bool& dataProcess, std::vector<std::string>& out) {
	if(data[0].back() == ':') { // new data
		data[0].pop_back();
		dataMapping[data[0]] = currentDataAddress;
		currentDataAddress += 4;
		out.emplace_back(convertDecimalToBinary(convertStringToNumber(data[2])));
	} else if(data[0] == ".word") {
		currentDataAddress += 4;
		out.emplace_back(convertDecimalToBinary(convertStringToNumber(data[1])));
	} else if(data[0] == ".text") {
		dataProcess = false;
	} else if(data[0] != ".data") { // non-define
		std::cout << "non-define input in the data process\n";
		for(auto s: data) {
			std::cout << s << ' ';
		}
		std::cout << '\n';
	}
}

void rInstruction(std::vector<std::string>& insData, TInstruction& ins, std::vector<std::string>& out) {
	if(ins.mnemonic == "jr") { /// special case
		int rs = convertStringToNumber(insData[1]);
		out.emplace_back(convertDecimalToBinary(ins.decimalForm | (rs << rsPosition)));
		return;
	}
	int rd = convertStringToNumber(insData[1]);
	int rs = convertStringToNumber(insData[2]);
	int rt = convertStringToNumber(insData[3]);
	int ret = ins.decimalForm | (rd << rdPosition);
	if(ins.mnemonic == "sll" || ins.mnemonic == "srl") {
		/// rs = rt, rt = shift amount
		ret |= (rt << shtPosition) | (rs << rtPosition);
	} else {
		ret |= (rt << rtPosition) | (rs << rsPosition);
	}
	out.emplace_back(convertDecimalToBinary(ret));
}

void iInstruction(std::vector<std::string>& insData, TInstruction& ins, std::vector<std::string>& out) {
	int rt = convertStringToNumber(insData[1]);
	int rs = 0, imm = 0, ret = ins.decimalForm;
	if(ins.mnemonic == "bne" || ins.mnemonic == "beq") {
		rs = rt;
		rt = convertStringToNumber(insData[2]);
		ret |= (rs << rsPosition) | (rt << rtPosition);
		if (insMapping.count(insData[3])) {
			imm = (insMapping[insData[3]] - currentPcAddress) / 4;
			ret |= trimBits(imm, rtPosition);
		} else {
			if(pendingIns.count(insData[3])) {
				pendingIns[insData[3]].emplace_back(out.size(), currentPcAddress);
			} else {
				pendingIns[insData[3]] = {{out.size(), currentPcAddress}};
			}
		}
		out.emplace_back(convertDecimalToBinary(ret));
	} else if(ins.mnemonic == "la") {
		/// lui + ori
		int val = dataMapping[insData[2]];
		imm = val >> rtPosition;
		out.emplace_back(convertDecimalToBinary(ret | (rt << rtPosition) | imm));
		imm <<= rtPosition;
		if(imm != val) {
			currentPcAddress += 4;
			TInstruction ori = *std::lower_bound(supportInstructions.begin(), supportInstructions.end(), 
							TInstruction("ori"));
			ret = ori.decimalForm | (rt << rtPosition) | (rt << rsPosition) | trimBits(val, rtPosition);
			out.emplace_back(convertDecimalToBinary(ret));
		}
	} else {
		imm = convertStringToNumber(insData.back());
		if(insData.size() > 3) rs = convertStringToNumber(insData[2]);
		ret |= (rs << rsPosition) | (rt << rtPosition) | trimBits(imm, rtPosition);
		out.emplace_back(convertDecimalToBinary(ret));
	}
}

void jInstruction(std::vector<std::string>& insData, TInstruction& ins, std::vector<std::string>& out) {
	int ret = ins.decimalForm;
	if (insMapping.count(insData[1])) {
		ret |= trimBits(insMapping[insData[1]] / 4, opcodePosition);
	} else {
		if(pendingIns.count(insData[1])) {
			pendingIns[insData[1]].emplace_back(out.size(), -1);
		} else {
			pendingIns[insData[1]] = {{out.size(), -1}};
		}
	}
	out.emplace_back(convertDecimalToBinary(ret));
}

void instructionProcessing(std::vector<std::string>& insts, std::vector<std::string>& out) {
	if(insts[0].back() == ':') {
		insts[0].pop_back();
		insMapping[insts[0]] = currentPcAddress;
		if(pendingIns.count(insts[0])) {
			auto& insInf = pendingIns[insts[0]];
			for(auto inf: insInf) {
				if(inf.second == -1) { /// jump ins
					out[inf.first].replace(instructionSize - opcodePosition, 
											opcodePosition, 
											convertDecimalToBinary(currentPcAddress / 4, opcodePosition));
				} else { /// beq, bne
					out[inf.first].replace(instructionSize - rtPosition, 
											rtPosition, 
											convertDecimalToBinary((currentPcAddress - inf.second) / 4, rtPosition));
				}
			}
			insInf.clear();
		}
	} else {
		currentPcAddress += 4;
		int insIdx = std::lower_bound(supportInstructions.begin(), supportInstructions.end(), 
							TInstruction(insts[0])) - supportInstructions.begin();
		if(insIdx >= supportInstructions.size() || supportInstructions[insIdx].mnemonic != insts[0]) 
			return;
		TInstruction ins = supportInstructions[insIdx];
		if (ins.format == 'R') {
			rInstruction(insts, ins, out);
		} else if(ins.format == 'I') {
			iInstruction(insts, ins, out);
		} else if(ins.format == 'J') {
			jInstruction(insts, ins, out);
		}
	}
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Usage: ./runfile <assembly file>\n"); //Example) ./runfile /sample_input/example1.s
		printf("Example) ./runfile ./sample_input/example1.s\n");
		exit(0);
	} else {
		std::string file = std::string(argv[1]);
		if(freopen(file.c_str(), "r", stdin) == 0) {
			printf("File open Error!\n");
			exit(1);
		}
		file.back() = 'o';
		freopen(file.c_str(), "w", stdout);

		std::sort(supportInstructions.begin(), supportInstructions.end());

		std::string cmdLine;
		std::vector<std::string> splitWords;
		std::vector<std::string> dataOutput, insOutput;
		bool dataProcess = true; /// current process is data or text
		while(getline(std::cin, cmdLine)) {
			splitInputLine(cmdLine, splitWords);
			if(splitWords.size() == 0) continue;
			if(dataProcess) {
				dataProcessing(splitWords, dataProcess, dataOutput);
			} else {
				instructionProcessing(splitWords, insOutput);
			}
		}

		std::cout << convertDecimalToBinary((int)insOutput.size() * 4)
				  << convertDecimalToBinary((int)dataOutput.size() * 4);
		for(auto s: insOutput) {
			std::cout << s;
		}
		for(auto s: dataOutput) {
			std::cout << s;
		}
		// std::cout << convertDecimalToBinary((int)insOutput.size() * 4) << '\n'
		// 		  << convertDecimalToBinary((int)dataOutput.size() * 4) << '\n';
		// for(auto s: insOutput) {
		// 	std::cout << s << '\n';
		// }
		// for(auto s: dataOutput) {
		// 	std::cout << s << '\n';
		// }
	}
	return 0;
}

