#ifndef CONTROL_CLASS
#define CONTROL_CLASS
#include <vector>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
using namespace std;
bool debug = true;
bool fileDebug = false;
// Control signals for the processor
struct control_t {


    bool reg_dest;           // 0 if rt, 1 if rd
    bool jump;               // 1 if jummp
    bool branchne;           //
    bool branch;             // 1 if branch
    bool mem_read;           // 1 if memory needs to be read
    bool mem_to_reg;         // 1 if memory needs to written to reg
    unsigned ALU_op : 2;     // 10 for R-type, 00 for LW/SW, 01 for BEQ/BNE, 11 for others
    bool mem_write;          // 1 if needs to be written to memory
    bool ALU_src;            // 0 if second operand is from reg_file, 1 if imm
    bool reg_write;          // 1 if need to write back to reg file

    int opcode;
//    int  rs;
//    int  rt;
//    int  rd;
    bool  shift;
    int  func_bits;
    bool jr;
    bool sh;
    bool jal;
//    int immediate;
//    int  address;
//    char type;

    map<pair<int, int>, vector<int>> instruction_control_map;


    void disp(uint32_t instruct){
        bitset<32> x(instruct);
        cout << " instruction: " << x << endl;
        int opcode = ((instruct>>26) & 63);
        int     rs = ((instruct>>21) & 31);
        int     rt = ((instruct>>16) & 31);
        int     rd = ((instruct>>11) & 31);
        int  shamt = ((instruct>>6) & 31);
        int  funct = (instruct & 63);
        int immediate = (instruct & 65535);
        int  address = (instruct & 67108863);
        cout << " opcode:" << opcode<< endl;
        cout << " rs:" <<rs << endl;
        cout << " rt:" <<rt << endl;
        cout << " rd:" <<rd << endl;
        cout << " shamt:" <<shamt<< endl;
        cout << " funct:" <<funct << endl;
        cout << " immediate:" <<immediate << endl;
        cout << " address:" <<address << endl;
    }

    void read_data(map<pair<int, int>, vector<int>> &instruction_control_map, string FILE_NAME)
    {
        fstream FILE(FILE_NAME);
        string line, data_from_file, instruction_name;

        getline(FILE, line); // purge col heads
        while(FILE.good())
        {
            // var town
            pair<int, int> op_func;
            vector<int> control_bits;

            getline(FILE, line);
            stringstream s(line);

            getline(s, data_from_file, ','); // purge instruction name
            if(debug) // just for debugging
                cout << data_from_file << endl;
            getline(s, data_from_file, ','); // purge instruction type

            getline(s, data_from_file, ','); // opcode
            op_func.first = stoi(data_from_file);

            getline(s, data_from_file, ','); // func
            op_func.second = stoi(data_from_file);

            if(debug) // just for debugging
                cout << op_func.first << " " << op_func.second << " ";

            while(s.good())
            {
                getline(s, data_from_file, ',');
                control_bits.push_back(stoi(data_from_file));
                if(debug) // just for debugging
                    cout << data_from_file << ",";
            }
            if(debug) // just for debugging
            {
                for (auto x: control_bits)
                    cout << x << " ";
            }
            instruction_control_map[op_func] = control_bits;
            cout << endl;
        }
        if(debug)
            cout << instruction_control_map[pair<int, int> (13,-1)][7];
    }

    // Prints the generated control signals
    void print()
    {
        cout << "REG_DEST: "   << reg_dest   << "\n";
        cout << "JUMP: "       << jump       << "\n";
        cout << "BRANCH: "     << branch     << "\n";
        cout << "MEM_READ: "   << mem_read   << "\n";
        cout << "MEM_TO_REG: " << mem_to_reg << "\n";
        cout << "ALU_OP: "     << ALU_op     << "\n";
        cout << "MEM_WRITE: "  << mem_write  << "\n";
        cout << "ALU_SRC: "    << ALU_src    << "\n";
        cout << "REG_WRITE: "  << reg_write  << "\n";


        if(fileDebug) {
            std::ofstream file;
            file.open("sim_out.txt", std::ios_base::app);

            file << "REG_DEST: "   << reg_dest   << "\n";
            file << "JUMP: "       << jump       << "\n";
            file << "BRANCH: "     << branch     << "\n";
            file << "MEM_READ: "   << mem_read   << "\n";
            file << "MEM_TO_REG: " << mem_to_reg << "\n";
            file << "ALU_OP: "     << ALU_op     << "\n";
            file << "MEM_WRITE: "  << mem_write  << "\n";
            file << "ALU_SRC: "    << ALU_src    << "\n";
            file << "REG_WRITE: "  << reg_write  << "\n";

            file.close();
        }

    }
    // TODO:
    // Decode instructions into control signals
    bool isValid(pair<int, int> op_func)
{
    bool valid = false;
    int O = op_func.first;
    int F = op_func.second;

    if(O == 0) {
        vector<int> rop = {32, 33, 36, 8, 39, 37, 0, 42, 43, 2, 34, 35};
        for (int i = 0; i < rop.size(); i++) {
            if (rop[i] == F) {
                valid = true;
            }
        }
    }

    if(F == -1) {
        vector<int> IJ_opcode = {2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 15, 35, 36, 37, 40, 41, 43, 48, 56};
        for (int i = 0; i < IJ_opcode.size(); i++) {
            if (IJ_opcode[i] != O) {
                valid = true;
            }
        }
    }

    return valid;
}
    void decode(uint32_t instruction)
    {

        if (debug)
            disp(instruction);
        // TODO: move var instantiation out of decode sense it loops
        // ----------------House keeping------------------------------
        opcode = ((instruction >> 26) & 63);
        func_bits = (opcode < 4) ? (instruction & 63) : -1;
        pair<int, int> op_func;
        op_func.first = opcode;
        op_func.second = func_bits;

        if (debug)
            cout << op_func.first << " " << op_func.second << endl;

        //----------------Control bit assignment--------------------
        // assigning the control bits the correct values from
        // the control vector
        // NOTE: this is where you'd add a new control line if needed'
        if(instruction != 0 && isValid(op_func))
        {
            reg_dest = instruction_control_map[op_func][0];
            jump = instruction_control_map[op_func][1];
            branch = instruction_control_map[op_func][2];
            branchne = instruction_control_map[op_func][3];
            mem_read = instruction_control_map[op_func][4];
            mem_to_reg = instruction_control_map[op_func][5];
            mem_write = instruction_control_map[op_func][6];
            ALU_op = instruction_control_map[op_func][7];
            ALU_src = instruction_control_map[op_func][8];
            reg_write = instruction_control_map[op_func][9];

            jr = instruction_control_map[op_func][10];
            shift = instruction_control_map[op_func][11];
            jal = instruction_control_map[op_func][12];

        }
        else {
            reg_dest = 0;
            jump = 0;
            branch = 0;
            branchne = 0;
            mem_read = 0;
            mem_to_reg = 0;
            mem_write = 0;
            ALU_op = 0;
            ALU_src = 0;
            reg_write = 0;
            jr = 0;
            shift = 0;
            jal = 0;
        }
    }
};
#endif
