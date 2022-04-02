#include <cstdint>
#include <iostream>
#include "memory.h"
#include "reg_file.h"
#include "ALU.h"
#include "control.h"
using namespace std;

//Tested:
//      toy
//      arthm
//      hidden
//      i extend and simple
//      r shift
//      jr
//      simple mem


//TODO: Update pc counter
void update_pc_counter()
{
    cout << "TODO: write PC counter update methods\n";
}


// just an if name MUX to match the cpu diagram for readability
uint32_t MUX(bool control_bit, uint32_t control_1, uint32_t control_0)
{
    return (control_bit) ? control_1 : control_0;
}

// Sample processor main loop for a single-cycle processor
void single_cycle_main_loop(Registers &reg_file, Memory &memory, uint32_t end_pc) {

    // Initialize ALU
    ALU alu;
    bool fileDebug = true;
    //Initialize Control
    control_t control = {.reg_dest = 0,
            .jump = 0,
            .branchne = 0,
            .branch = 0,
            .mem_read = 0,
            .mem_to_reg = 0,
            .ALU_op = 0,
            .mem_write = 0,
            .ALU_src = 0,
            .reg_write = 0,
            .opcode = 0,
            .func_bits = 0};
    control.read_data(control.instruction_control_map, "data.txt"); // import control bit mapping

    uint32_t num_cycles = 0;
    uint32_t num_instrs = 0;

    //-------------Vars for mem/reg read/write-------------------
    uint32_t reg_data_1; // var for reg_file access to put data in
    uint32_t reg_data_2; // var for reg_file access to put data in
    uint32_t   mem_data; // var for memory access to store data in
    uint32_t   ALU_zero; // alu zero flag


    //-------------Instruction vars-------------------------------
    uint32_t instruction; // [31-0] // whole instruction

    uint32_t  opcode; //[31-26]
    uint32_t      rs; //[25-11]
    uint32_t      rt; //[20-16]
    uint32_t      rd; // [15-11]
    uint32_t    shamt; // [10-6]
    uint32_t    func; // [5-0]
    uint32_t address; // [25-0]
    int immediate; // [15-0] (short casing for sign extension)

    ofstream file;
    file.open("sim_out.txt");
    file << "";
    file.close();
    //--------------single cycle processor main loop--------------------------------------------
    while (reg_file.pc != end_pc)
    {
        //memory.print(0, 10);
        //--------------------Fetch----------------------------------
        // Retrieve instruction from memory
        memory.access(reg_file.pc, instruction, 0, 1, 0);
        reg_file.pc += 4;
        //--------------------Decode---------------------------------
        // decode control bit (control unit)
        control.decode(instruction);
        control.print(); // used for autograding
        // assign parts of instruction to vars
        opcode = ((instruction>>26) & 63); //[31-26]
        rs = ((instruction>>21) & 31); //[25-11]
        rt = ((instruction>>16) & 31); //[20-16]
        rd = ((instruction>>11) & 31); // [15-11]
        shamt = ((instruction>>6) & 31); // [10-6]
        func = (instruction & 63);      // [5-0]
        immediate = (short)(instruction & 0x0000ffff);//(instruction & 0x8000) ? ((instruction & 0x0000ffff) ^ 0xffff0000) : (instruction & 0x0000ffff); // [15-0] (short casing for sign extention)
        address = (instruction & 67108863); // [25-0]


        //--------------------Execute-------------------------------
        // Read from reg file
        reg_file.access(rs, rt, reg_data_1, reg_data_2, MUX(control.reg_dest, rd, rt), 0, 0);
        if (debug) { cout << "regD1: " << (int) reg_data_1 << " regD2: " << (int) reg_data_2 << endl; }
        // bullshit alu gen/EX
        alu.generate_control_inputs(control.ALU_op, func, opcode);
        uint32_t alu_result = alu.execute(reg_data_1, MUX(control.ALU_src, immediate, reg_data_2), ALU_zero);
        if(debug) {
            cout << "immediate: " << (int)immediate<< endl;
            cout << "ALUOUT: " << (int) alu_result << endl;
            cout << "alu mux: "<< (int)MUX(control.ALU_src, immediate, reg_data_2) << endl;
            cout << "alu mux inv: "<<(int) MUX(!control.ALU_src, immediate, reg_data_2) << endl;
        }
        //--------------------Store----------------------------------
        // Memory
        if(opcode == 41) // store half word jank
        {
            memory.access(alu_result*4, mem_data, reg_data_2, 1, 0);
            reg_data_2 = (reg_data_2 & 0x0000ffff) | (mem_data & 0xffff0000);
        }
        if(opcode == 40) // store half byte jank
        {
            memory.access(alu_result*4, mem_data, reg_data_2, 1, 0);
            reg_data_2 = (reg_data_2 & 0x000000ff) | (mem_data & 0xffffff00);
        } // store half word and store byte
        memory.access(alu_result*4, mem_data, reg_data_2, control.mem_read, control.mem_write);
        //cout << "This is memdata out" << reg_data_2 << endl;
        if(opcode == 37) // load half unsigned
        {
            mem_data &= 0x0000ffff;
        }
        if(opcode == 36) // load byte i
        {
            mem_data &= 0x000000ff;
        }
        if((opcode == 0) && (func == 0)) // shifts
        {
            alu_result = reg_data_2 << shamt;
        }
        if((opcode == 0) && (func == 2))
        {
            alu_result = reg_data_2 >> shamt;
        } //shifts
        // Write Back
        reg_file.access(rs, rt, reg_data_1, reg_data_2, MUX(control.reg_dest, rd, rt), control.reg_write, MUX(control.mem_to_reg, mem_data, alu_result));
        //memory.print(100, 10);
        //-------------------PC Path---------------------------------

        // TODO: make better jump handler
        // junk jump handler
        if(control.jump)
        {
            switch(opcode)
            {
                case 0:
                    reg_file.pc = rs<<2;
                    break;
                case 2:
                    reg_file.pc = address<<2;
                    break;
                case 3:
                    reg_file.access(0, 0, reg_data_1, reg_data_2, 31, true, (reg_file.pc+8));
                    break;
            }
        }

        if((opcode == 0) && (func == 8)) // jr bullshit
        {
            reg_file.pc = reg_data_1;
        }

        // TODO: make cleaner and to execute stage
        // branch handler
        cout << "Program Counter: " << reg_file.pc << endl;
        if (opcode == 4 && ALU_zero) // janky branch equal
        {
            reg_file.pc += immediate * 4;
        }
        if (opcode == 5 && !ALU_zero) // janky branch not equal
        {
            reg_file.pc += immediate * 4;
        }
        //------------------House Keeping nothing functional---------
        if(fileDebug)
        {
            file.open("sim_out.txt", std::ios_base::app);
            file << "CYCLE" << num_cycles << "\n";
            file.close();
        }
        cout << "CYCLE" << num_cycles << "\n";
        cout << "pc: " << reg_file.pc << endl;
        reg_file.print(); // used for automated testing

        if(num_cycles > 9 && 0)
            break;
        num_cycles++;
        num_instrs++;
    }
    if(fileDebug)
    {
        file.open("sim_out.txt", std::ios_base::app);
        file << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";
        file.close();

    }
    cout << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";

}



void pipelined_main_loop(Registers &reg_file, Memory &memory, uint32_t end_pc) {
    uint32_t num_cycles = 0;
    uint32_t num_instrs = 0; 

    while (true) {

        cout << "CYCLE" << num_cycles << "\n";

        //reg_file.print(); // used for automated testing

        num_cycles++;

        //num_instrs += committed_insts;
        if (1) {
            break;
        }
    }

    cout << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";
}

void speculative_main_loop(Registers &reg_file, Memory &memory, uint32_t end_pc) {
    uint32_t num_cycles = 0;
    uint32_t num_instrs = 0; 

    while (true) {

        cout << "CYCLE" << num_cycles << "\n";

        reg_file.print(); // used for automated testing

        num_cycles++;

        //num_instrs += committed_insts;
        if (1) {
            break;
        }
    }
    cout << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";
}

void io_superscalar_main_loop(Registers &reg_file, Memory &memory, uint32_t end_pc) {
    uint32_t num_cycles = 0;
    uint32_t num_instrs = 0; 

    while (true) {

        cout << "CYCLE" << num_cycles << "\n";

        reg_file.print(); // used for automated testing

        num_cycles++;

        //num_instrs += committed_insts;
        if (1) {
            break;
        }
    }

    cout << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";
}

void ooo_scalar_main_loop(Registers &reg_file, Memory &memory, uint32_t end_pc) {
    uint32_t num_cycles = 0;
    uint32_t num_instrs = 0; 

    while (true) {

        cout << "CYCLE" << num_cycles << "\n";

        //reg_file.print(); // used for automated testing

        num_cycles++;

        //num_instrs += committed_insts;
        if (1) {
            break;
        }
    }

    cout << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";
}