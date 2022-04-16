#ifndef STATE
#define STATE
#include <vector>
#include <cstdint>
#include <iostream>
#include "control.h"
//
// Pipeline registers implementation
//
// TODO:
// IFID Pipeline register, only contains instruction and pc + 1 (aka plus 4)
// also contains the jump register
struct IFID {
    bool stall;
    uint32_t instruction;
    uint32_t pc;
    uint32_t jump_pc;

};

// TODO:
// IDEX Pipeline register
struct IDEX {
    bool stall;
    control_t control;
    //uint32_t instruction; this should already be processed
    uint32_t pc;
    uint32_t read_data_1;
    uint32_t read_data_2;
    uint32_t sign_extended;
    uint32_t opcode;
    //uint32_t rt;
    //uint32_t rd;
    // we only need the one write register
    uint32_t r_write; //I prefer write but this is consistent with previous name convention
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
    control_t control;
    uint32_t pc_adder;
    uint32_t alu_zero;
    uint32_t alu_result;
    uint32_t read_data_2;
    uint32_t r_write;
    uint32_t sign_extended;
    uint32_t funct_bits; // to check for loads etc.

    uint32_t jump_pc;
    uint32_t pc_alu_result;
    //uint32_t write_num; redundant
};

// TODO:
// MEMWB Pipwline register
struct MEMWB {
    bool stall;
    control_t control;
    uint32_t read_data;
    uint32_t alu_result;
    uint32_t r_write;
    uint32_t jal_reg;
  //  uint32_t write_num; redundant
};

#endif
