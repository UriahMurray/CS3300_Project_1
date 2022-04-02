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
// IFID Pipeline register, only contains instruction and pc + 1
struct IFID {
    bool stall;
    uint32_t instruction;
    int pc;

};

// TODO:
// IDEX Pipeline register
struct IDEX {
    bool stall;
    control_t control;
    uint32_t instruction;
    int pc;
    uint32_t read_data_1;
    uint32_t read_data_2;
    uint32_t sign_extended;
    uint32_t rt;
    uint32_t rd;
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
};

// TODO:
// MEMWB Pipwline register
struct MEMWB {
    bool stall;
    control_t control;
    uint32_t read_data;
    uint32_t alu_result;
    uint32_t reg_rd_rt;
};

#endif
