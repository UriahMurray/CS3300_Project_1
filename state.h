#ifndef STATE
#define STATE
#include <vector>
#include <cstdint>
#include <iostream>
#include "control.h"
using namespace std;
//
// Pipeline registers implementation
//
// TODO:
// IFID Pipeline register, only contains instruction and pc + 1 (aka plus 4)
// also contains the jump register
struct IFID {
    bool stall;
    bool valid;
    uint32_t instruction;
    uint32_t pc;
    uint32_t jump_pc;

};

// TODO:
// IDEX Pipeline register
struct IDEX {
    bool stall;
    bool valid;
    bool last;
    control_t control;
    //uint32_t instruction; this should already be processed
    uint32_t pc;
    uint32_t read_data_1;
    uint32_t read_data_2;
    int sign_extended;
    uint32_t opcode;
    //uint32_t rt;
    //uint32_t rd;
    // we only need the one write register
    uint32_t r_write;
    uint32_t funct_bits; // for alu op
    //uint32_t shamt;
    uint32_t jump_pc;
    uint32_t rs_num;
    uint32_t rt_num; // this is the register number we're writing to
    uint32_t rd_num;


};

// TODO:
// EXMEM Pipeline register
struct EXMEM {
    bool stall;
    bool valid;
    bool last;
    control_t control;
    uint32_t pc_adder;
    uint32_t alu_zero;
    uint32_t alu_result;
    uint32_t read_data_2;
    uint32_t r_write;
    int sign_extended;
    uint32_t funct_bits; // to check for loads etc.
    uint32_t opcode;

    uint32_t jump_pc;
    uint32_t pc_alu_result;
    //uint32_t write_num; redundant
};

// TODO:
// MEMWB Pipwline register
struct MEMWB {
    bool stall;
    bool valid;
    control_t control;
    uint32_t read_data;
    uint32_t alu_result;
    uint32_t r_write;
    uint32_t jal_reg;
    uint32_t opcode;
    uint32_t pc_adder;
    uint32_t funct_bits;

  //  uint32_t write_num; redundant
};

//void print_memwb()
//{
//    cout << "~~~~~~~~~ v MEMWB CURRENT STATE v ~~~~~~~~~" << endl;
//    cout << "stall      : " <<   MEMWB.stall      << endl;
//    cout << "valid      : " <<   MEMWB.valid      << endl;
//    cout << "control    : " <<   MEMWB.control    << endl;
//    cout << "read_data  : " <<   MEMWB.read_data  << endl;
//    cout << "alu_result : " <<   MEMWB.alu_result << endl;
//    cout << "r_write    : " <<   MEMWB.r_write    << endl;
//    cout << "jal_reg    : " <<   MEMWB.jal_reg    << endl;
//    cout << "opcode     : " <<   MEMWB.opcode     << endl;
//    cout << "~~~~~~~~~ ^ MEMWB CURRENT STATE ^ ~~~~~~~~~" << endl;
//}

#endif
