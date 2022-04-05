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
    int pc;
    uint32_t jump_pc;

};

// TODO:
// IDEX Pipeline register
struct IDEX {
    bool stall;
    control_t control;
    //uint32_t instruction; this should already be processed
    int pc;
    uint32_t read_data_1;
    uint32_t read_data_2;
    uint32_t sign_extended;
    //uint32_t rt;
    //uint32_t rd;
    // we only need the one write register
    uint32_t reg_rd_rt; //I prefer write but this is consistent with previous name convention
    uint32_t funct_bits; // for alu op
    uint32_t shamt;  
    uint32_t jump_pc;
    
    
};

// TODO:
// EXMEM Pipeline register
struct EXMEM {
    bool stall;
    control_t control;
    int pc_adder;
    bool alu_zero;
    uint32_t alu_result;
    uint32_t read_data_2;
    uint32_t reg_rd_rt;
    uint32_t sign_extended;
    uint32_t funct_bits; // to check for loads etc.
    
    uint32_t jump_pc;
    uint32_t pc_alu_result;
};

// TODO:
// MEMWB Pipwline register
struct MEMWB {
    bool stall;
    control_t control;
    uint32_t read_data;
    uint32_t alu_result;
    uint32_t reg_rd_rt;
    uint32_t jal_reg;
};

#endif
