#include <cstdint>
#include <iostream>
#include "memory.h"
#include "reg_file.h"
#include "ALU.h"
#include "control.h"
#include "state.h"
#include "BPU.h"
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
            .shift = 0,
            .func_bits = 0};
    control.read_data(control.instruction_control_map, "data.txt"); // import control bit mapping

    uint32_t num_cycles = 0;
    uint32_t num_instrs = 0;

    //-------------Vars for mem/reg read/write-------------------
    uint32_t reg_data_1; // var for reg_file access to put data in
    uint32_t reg_data_2; // var for reg_file access to put data in
    uint32_t   mem_data; // var for memory access to store data in
    uint32_t   ALU_zero; // alu zero flag

    //--------------ALU VARS--------------------------------------
    uint32_t alu_op1;
    uint32_t  alu_op2;

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

        // Read from reg file
        reg_file.access(rs, rt, reg_data_1, reg_data_2, MUX(control.reg_dest, rd, rt), 0, 0);
        if (debug) { cout << "regD1: " << (int) reg_data_1 << " regD2: " << (int) reg_data_2 << endl; }

        //--------------------Execute-------------------------------

        // bullshit alu gen/EX
        alu.generate_control_inputs(control.ALU_op, func, opcode);

        alu_op1 = MUX(control.shift, shamt, reg_data_1);

        alu_op2 = MUX(control.ALU_src, immediate, reg_data_2);



        uint32_t alu_result = alu.execute(alu_op1, alu_op2, ALU_zero);
        if(debug) {
            cout << "immediate: " << (int)immediate<< endl;
            cout << "ALUOUT: " << (int) alu_result << endl;
            cout << "alu mux: "<< (int)MUX(control.ALU_src, immediate, reg_data_2) << endl;
            cout << "alu mux inv: "<<(int) MUX(!control.ALU_src, immediate, reg_data_2) << endl;
        }
        //--------------------Store----------------------------------
        // Memory

        // load word or store word
        memory.access(alu_result*4+400, mem_data, reg_data_2, control.mem_read, control.mem_write);
        //cout << "This is memdata out" << reg_data_2 << endl;
        if(opcode == 37) // load half unsigned
        {
            mem_data &= 0x0000ffff;
        }
        if(opcode == 36) // load byte i
        {
            mem_data &= 0x000000ff;
        }
//        if((opcode == 0) && (func == 0)) // shifts
//        {
//            alu_result = reg_data_2 << shamt;
//        }
//        if((opcode == 0) && (func == 2))
//        {
//            alu_result = reg_data_2 >> shamt;
//        } //shifts
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
            reg_file.pc += immediate   * 4;
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
    // Initialize ALU
    ALU alu;
    bool fileDebug = true;
    bool end = false;
    bool stallhaz = false;
    bool noFmemwb = false;
    bool noFexmem = false;
    uint32_t num_cycles = 0;
    uint32_t num_instrs = 0;
    uint32_t bogus = 0;
    struct IFID rIFID;
    rIFID.stall = false;
    rIFID.valid = false;
    rIFID.pc = 0;
    rIFID.jump_pc = 0;
    struct IDEX rIDEX;
    rIDEX.stall = false;
    rIDEX.valid = false;
    //Init control
    rIDEX.control.reg_dest = 0;
    rIDEX.control.jump = 0;
    rIDEX.control.branchne = 0;
    rIDEX.control.branch = 0;
    rIDEX.control.mem_read = 0;
    rIDEX.control.mem_to_reg = 0;
    rIDEX.control.ALU_op = 0;
    rIDEX.control.mem_write = 0;
    rIDEX.control.ALU_src = 0;
    rIDEX.control.reg_write = 0;
    rIDEX.control.opcode = 0;
    rIDEX.control.shift = 0;
    rIDEX.control.func_bits = 0;
    rIDEX.control.jal = 0;
    rIDEX.control.read_data(rIDEX.control.instruction_control_map, "data.txt"); // import control bit mapping
    struct EXMEM rEXMEM;
    rEXMEM.stall = false;
    rEXMEM.valid = false;
    // init control
    rEXMEM.control.reg_dest = 0;
    rEXMEM.control.jump = 0;
    rEXMEM.control.branchne = 0;
    rEXMEM.control.branch = 0;
    rEXMEM.control.mem_read = 0;
    rEXMEM.control.mem_to_reg = 0;
    rEXMEM.control.ALU_op = 0;
    rEXMEM.control.mem_write = 0;
    rEXMEM.control.ALU_src = 0;
    rEXMEM.control.reg_write = 0;
    rEXMEM.control.opcode = 0;
    rEXMEM.control.shift = 0;
    rEXMEM.control.func_bits = 0;
    rEXMEM.control.jal = 0;
    rEXMEM.control.read_data(rEXMEM.control.instruction_control_map, "data.txt");
    struct MEMWB rMEMWB;
    rMEMWB.stall = false;
    rMEMWB.valid = false;
    // init control
    rMEMWB.control.reg_dest = 0;
    rMEMWB.control.jump = 0;
    rMEMWB.control.branchne = 0;
    rMEMWB.control.branch = 0;
    rMEMWB.control.mem_read = 0;
    rMEMWB.control.mem_to_reg = 0;
    rMEMWB.control.ALU_op = 0;
    rMEMWB.control.mem_write = 0;
    rMEMWB.control.ALU_src = 0;
    rMEMWB.control.reg_write = 0;
    rMEMWB.control.opcode = 0;
    rMEMWB.control.shift = 0;
    rMEMWB.control.func_bits = 0;
    rMEMWB.control.jal = 0;
    rMEMWB.control.read_data(rMEMWB.control.instruction_control_map, "data.txt");
    uint32_t dummy = 0;
    while (!end) {
        num_cycles++;
        stallhaz = false;
        rIDEX.stall = false;
        rIFID.stall = false;
        if ((reg_file.pc) == end_pc+16)
        {
            end = true;
        }
        // forwarding happens here needs cleaning

         if (rIDEX.rs_num == rMEMWB.r_write)
         {
           if (rMEMWB.opcode == 0x5 || rMEMWB.opcode == 0x4 || rMEMWB.opcode == 0x3 || rMEMWB.opcode == 0x2 || rMEMWB.opcode == 0x28 || rMEMWB.opcode == 0x29 || rMEMWB.opcode == 0x38 || rMEMWB.opcode == 0x2b || (rMEMWB.opcode == 0x0 && rMEMWB.funct_bits == 0x8))
           {
             noFmemwb = true;
           }else
           {
             noFmemwb = false;
           }
           if (!rIDEX.control.shift)
           {
             if (1/*rMEMWB.valid*/)
             {
               if (!noFmemwb)
               {
                 if (rMEMWB.control.mem_to_reg)
                 {
                   rIDEX.read_data_1 = rMEMWB.read_data;
                 }else
                 {
                   rIDEX.read_data_1 = rMEMWB.alu_result;
                 }
               }
             }
           }
         }
         if (rIDEX.rt_num == rMEMWB.r_write)
         {
           if (rMEMWB.opcode == 0x5 || rMEMWB.opcode == 0x4 || rMEMWB.opcode == 0x3 || rMEMWB.opcode == 0x2 || rMEMWB.opcode == 0x28 || rMEMWB.opcode == 0x29 || rMEMWB.opcode == 0x38 || rMEMWB.opcode == 0x2b || (rMEMWB.opcode == 0x0 && rMEMWB.funct_bits == 0x8))
           {
             noFmemwb = true;
           }else
           {
             noFmemwb = false;
           }
           if (1/*rMEMWB.valid*/)
           {
             if (!noFmemwb)
             {
               if (rMEMWB.control.mem_to_reg)
               {
                 rIDEX.read_data_2 = rMEMWB.read_data;
               }else
               {
                 rIDEX.read_data_2 = rMEMWB.alu_result;
               }
             }
           }
         }

         if (rIDEX.rs_num == rEXMEM.r_write)
         {
           if (rEXMEM.opcode == 0x5 || rEXMEM.opcode == 0x4 || rEXMEM.opcode == 0x3 || rEXMEM.opcode == 0x2 || rEXMEM.opcode == 0x28 || rEXMEM.opcode == 0x29 || rEXMEM.opcode == 0x38 || rEXMEM.opcode == 0x2b || (rEXMEM.opcode == 0x0 && rEXMEM.funct_bits == 0x8))
           {
             noFexmem = true;
           }else
           {
             noFexmem = false;
           }
           if (!rIDEX.control.shift)
           {
             if (1/*rEXMEM.valid*/)
             {
               if (!noFexmem)
               {
                 if (rEXMEM.opcode == 0x24 || rEXMEM.opcode == 0x25 || rEXMEM.opcode == 0x30 || rEXMEM.opcode == 0xf || rEXMEM.opcode == 0x23)
                 {
                   rIDEX.stall = true;
                   rIFID.stall = true;
                   stallhaz = true;
                 }else
                 {
                   rIDEX.read_data_1 = rEXMEM.alu_result;
                 }
               }
             }
           }
         }
         if (rIDEX.rt_num == rEXMEM.r_write)
         {
           if (rEXMEM.opcode == 0x5 || rEXMEM.opcode == 0x4 || rEXMEM.opcode == 0x3 || rEXMEM.opcode == 0x2 || rEXMEM.opcode == 0x28 || rEXMEM.opcode == 0x29 || rEXMEM.opcode == 0x38 || rEXMEM.opcode == 0x2b || (rEXMEM.opcode == 0x0 && rEXMEM.funct_bits == 0x8))
           {
             noFexmem = true;
           }else
           {
             noFexmem = false;
           }
             if (1/*rEXMEM.valid*/)
             {
               if (!noFexmem)
               {
                 if (rEXMEM.opcode == 0x24 || rEXMEM.opcode == 0x25 || rEXMEM.opcode == 0x30 || rEXMEM.opcode == 0xf || rEXMEM.opcode == 0x23)
                 {
                   rIDEX.stall = true;
                   rIFID.stall = true;
                   stallhaz = true;
                 }else
                 {
                   rIDEX.read_data_2 = rEXMEM.alu_result;
                 }
               }
             }
         }

        //This is the write back stage of things
        if (rMEMWB.valid)
        {
          num_instrs++;
        }

        //~~~~~~~~~~~~~~~~~~~~~LOAD HALF WORDS AND BYTE MASKING~~~~~~~~~~~~~~~~~~~~~
        if(rMEMWB.opcode == 37) // load half unsigned
        {
          rMEMWB.read_data &= 0x0000ffff;
        }
        if(rMEMWB.opcode == 36) // load byte i
        {
          rMEMWB.read_data &= 0x000000ff;
        }
        if (false/*rMEMWB.control.jal*/)
        {
          //  reg_file.access(dummy, dummy, dummy, dummy, 0x1f, true, rMEMWB.jal_reg);
        }else if (rMEMWB.control.mem_to_reg)
        {
            reg_file.access(dummy, dummy, dummy, dummy, rMEMWB.r_write, rMEMWB.control.reg_write, rMEMWB.read_data);
        }else
        {
          if (!rMEMWB.control.mem_write)
          {
            reg_file.access(dummy, dummy, dummy, dummy, rMEMWB.r_write, rMEMWB.control.reg_write, rMEMWB.alu_result);
          }
        }
      //  if(rMEMWB.opcode == 37) // load half unsigned
        //{
            //this var need to be read_data mem_data &= 0x0000ffff;
      //  }
        //if(rMEMWB.opcode == 36) // load byte i
      //  {
            //this var need to be read_data mem_data &= 0x000000ff;
      //  }

      // This is the mem stage of things
        if(!rMEMWB.stall)
        {
            //~~~~~~~~~~~~~~~~~~~~~~~~PASS VAULES: EXME->MEMWB~~~~~~~~~~~~~~~~~~~~~~~~
            rMEMWB.control = rEXMEM.control;
            rMEMWB.alu_result = rEXMEM.alu_result;
            rMEMWB.r_write = rEXMEM.r_write;
            rMEMWB.jal_reg = rEXMEM.pc_adder + 4;
            rMEMWB.opcode = rEXMEM.opcode;
            rMEMWB.valid = rEXMEM.valid;
            rMEMWB.pc_adder = rEXMEM.pc_adder;
            rMEMWB.funct_bits = rEXMEM.funct_bits;
            //rMEMWB.read_data = 1; //gonna need some help here: not need value set by being passed by reference
            //~~~~~~~~~~~~~~~~~~~~~STORE HALF WOES AND BYTE LOGIC~~~~~~~~~~~~~~~~~~~~~
            if(rMEMWB.opcode == 0x29) // store half word jank
            {
                memory.access(rEXMEM.alu_result*4+400, bogus, rEXMEM.read_data_2, 1, 0);
                rEXMEM.read_data_2 = (rEXMEM.read_data_2 & 0x0000ffff) | (bogus & 0xffff0000);
                memory.access(rEXMEM.alu_result*4+400, rMEMWB.read_data, rEXMEM.read_data_2, 0, 1);
            }else if(rMEMWB.opcode == 0x28) // store half byte jank
            {
                memory.access(rEXMEM.alu_result*4+400, bogus, rEXMEM.read_data_2, 1, 0);
                rEXMEM.read_data_2 = (rEXMEM.read_data_2 & 0x000000ff) | (bogus & 0xffffff00);
                memory.access(rEXMEM.alu_result*4+400, rMEMWB.read_data, rEXMEM.read_data_2, 0, 1);
            }else
            {
                memory.access( rEXMEM.alu_result*4+400, rMEMWB.read_data, rEXMEM.read_data_2, rEXMEM.control.mem_read, rEXMEM.control.mem_write);
            }
        }


        // This is the EX stage of things
        if(!rEXMEM.stall)
        {
          if (!stallhaz)
          {
            rEXMEM.valid = rIDEX.valid;
            rEXMEM.control = rIDEX.control;
            rEXMEM.pc_alu_result = rIDEX.pc + rIDEX.sign_extended*4;
            rEXMEM.pc_adder = rIDEX.pc;
            rEXMEM.read_data_2 = rIDEX.read_data_2;
            rEXMEM.r_write = rIDEX.r_write;
            rEXMEM.sign_extended = rIDEX.sign_extended;
            rEXMEM.funct_bits = rIDEX.funct_bits;
            rEXMEM.jump_pc = rIDEX.jump_pc;
            rEXMEM.opcode = rIDEX.opcode;
            alu.generate_control_inputs(rIDEX.control.ALU_op, rIDEX.funct_bits, rIDEX.opcode);
            if (!rIDEX.control.ALU_src)
            {
              rEXMEM.alu_result = alu.execute(rIDEX.read_data_1, rIDEX.read_data_2, rEXMEM.alu_zero);
            }
            else
            {
              rEXMEM.alu_result = alu.execute(rIDEX.read_data_1, rIDEX.sign_extended, rEXMEM.alu_zero);

            }
          }else
          {
            rEXMEM.valid = false;
            rEXMEM.control.decode(0);
            rEXMEM.pc_alu_result = 0;
            rEXMEM.pc_adder = 0;
            rEXMEM.read_data_2 = 0;
            rEXMEM.r_write = 0;
            rEXMEM.sign_extended = 0;
            rEXMEM.funct_bits = 0;
            rEXMEM.jump_pc = 0;
            rEXMEM.opcode = 0;
            alu.generate_control_inputs(rEXMEM.control.ALU_op, 0, 0);
            rEXMEM.alu_result = 0;
            //cout << "I helped";
          }
        }


        // This is the ID stage of things
        if(!rIDEX.stall)
        {
            //~~~~~~~~~~~~~~~DECODING CONTROL BITS~~~~~~~~~~~~~~~~~~~~
            rIDEX.control.decode(rIFID.instruction); // decode control bits
            //~~~~~~~~~~~~~~~~PASS VALUES: IFID->IDEX~~~~~~~~~~~~~~~~
            rIDEX.pc = rIFID.pc;
            rIDEX.valid = rIFID.valid;
            rIDEX.jump_pc = rIFID.jump_pc;
            //rIDEX.read_data_2 = ((rIFID.instruction>>16) & 31);
            rIDEX.sign_extended = (short)(rIFID.instruction & 0x0000ffff);
            if (!rIDEX.control.reg_dest)
            {
              rIDEX.r_write = ((rIFID.instruction>>16) & 31);
            }else
            {
              rIDEX.r_write = ((rIFID.instruction>>11) & 31);
            }
            rIDEX.funct_bits = (rIFID.instruction & 63);
            rIDEX.rs_num = ((rIFID.instruction>>0x15) & 0x1F);
            rIDEX.rt_num = ((rIFID.instruction>>0x10) & 0x1F); //[25-11]
            rIDEX.rd_num = ((rIFID.instruction>>0xB) & 0x1F);
            rIDEX.opcode = ((rIFID.instruction>>26) & 63);

        //    rIDEX.shamt = ((rIFID.instruction>>0x6) & 0x1F); no longer needed
            //~~~~~~~~~~~~~~~REG FILE ACCESS~~~~~~~~~~~~~~~~~~~~~~~~~~
            reg_file.access(rIDEX.rs_num, rIDEX.rt_num, rIDEX.read_data_1, rIDEX.read_data_2, MUX(rIDEX.control.reg_dest, rIDEX.rd_num, rIDEX.rt_num), 0, 0);
            if (rIDEX.control.shift) // if this is a shift, the alu needs the shamt instead of rs
            {
              rIDEX.read_data_1 = ((rIFID.instruction>>0x6) & 0x1F);
            }
            if(rIDEX.control.jump)
            {
              switch(rIDEX.opcode)
              {
                  case 0:
                      reg_file.pc = rIDEX.read_data_1<<2;
                      break;
                  case 2:
                      reg_file.pc = rIDEX.jump_pc<<2;
                      break;
                  case 3:
                      //reg_file.access(0, 0, reg_data_1, reg_data_2, 31, true, (reg_file.pc+8));
                      reg_file.pc = rIDEX.jump_pc<<2;
                      break;
              }
            }
          if (rEXMEM.opcode == 4 && rEXMEM.alu_zero) // janky branch equal
          {
              // reg_file.pc += immediate * 4; not sure
              reg_file.pc = rEXMEM.pc_alu_result;
              rIFID.valid = false;
              rIFID.instruction = 0;
              rIDEX.stall = true;
              rIFID.stall = true;
          }
          if (rEXMEM.opcode == 5 && !rEXMEM.alu_zero) // janky branch not equal
          {
              //reg_file.pc += immediate * 4; not sure
              reg_file.pc = rEXMEM.pc_alu_result;
              rIFID.valid = false;
              rIFID.instruction = 0;
              rIDEX.stall = true;
              rIFID.stall = true;
          }
        }

        // This is the If stage of things
        if(!rIFID.stall)
        {
          if (rIFID.instruction != 0)
          {
            rIFID.valid = true;
          }else
          {
            rIFID.valid = false;
          }
          memory.access(reg_file.pc, rIFID.instruction, 0, 1, 0);
          reg_file.pc += 4;
          rIFID.pc = reg_file.pc;
          rIFID.jump_pc = (rIFID.instruction & 67108863);
        }




        cout << "CYCLE" << num_cycles << "\n";

        reg_file.print(); // used for automated testing
        //num_instrs += committed_insts;
    }

    cout << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";
}

void speculative_main_loop(Registers &reg_file, Memory &memory, uint32_t end_pc) {
  // Initialize ALU
  ALU alu;
  BPU bpu;
  bool fileDebug = true;
  bool end = false;
  bool stallhaz = false;
  bool noFmemwb = false;
  bool noFexmem = false;
  uint32_t num_cycles = 0;
  uint32_t num_instrs = 0;
  uint32_t bogus = 0;
  struct IFID rIFID;
  rIFID.stall = false;
  rIFID.valid = false;
  rIFID.pc = 0;
  rIFID.jump_pc = 0;
  struct IDEX rIDEX;
  rIDEX.stall = false;
  rIDEX.valid = false;
  //Init control
  rIDEX.control.reg_dest = 0;
  rIDEX.control.jump = 0;
  rIDEX.control.branchne = 0;
  rIDEX.control.branch = 0;
  rIDEX.control.mem_read = 0;
  rIDEX.control.mem_to_reg = 0;
  rIDEX.control.ALU_op = 0;
  rIDEX.control.mem_write = 0;
  rIDEX.control.ALU_src = 0;
  rIDEX.control.reg_write = 0;
  rIDEX.control.opcode = 0;
  rIDEX.control.shift = 0;
  rIDEX.control.func_bits = 0;
  rIDEX.control.jal = 0;
  rIDEX.control.read_data(rIDEX.control.instruction_control_map, "data.txt"); // import control bit mapping
  struct EXMEM rEXMEM;
  rEXMEM.stall = false;
  rEXMEM.valid = false;
  // init control
  rEXMEM.control.reg_dest = 0;
  rEXMEM.control.jump = 0;
  rEXMEM.control.branchne = 0;
  rEXMEM.control.branch = 0;
  rEXMEM.control.mem_read = 0;
  rEXMEM.control.mem_to_reg = 0;
  rEXMEM.control.ALU_op = 0;
  rEXMEM.control.mem_write = 0;
  rEXMEM.control.ALU_src = 0;
  rEXMEM.control.reg_write = 0;
  rEXMEM.control.opcode = 0;
  rEXMEM.control.shift = 0;
  rEXMEM.control.func_bits = 0;
  rEXMEM.control.jal = 0;
  rEXMEM.control.read_data(rEXMEM.control.instruction_control_map, "data.txt");
  struct MEMWB rMEMWB;
  rMEMWB.stall = false;
  rMEMWB.valid = false;
  // init control
  rMEMWB.control.reg_dest = 0;
  rMEMWB.control.jump = 0;
  rMEMWB.control.branchne = 0;
  rMEMWB.control.branch = 0;
  rMEMWB.control.mem_read = 0;
  rMEMWB.control.mem_to_reg = 0;
  rMEMWB.control.ALU_op = 0;
  rMEMWB.control.mem_write = 0;
  rMEMWB.control.ALU_src = 0;
  rMEMWB.control.reg_write = 0;
  rMEMWB.control.opcode = 0;
  rMEMWB.control.shift = 0;
  rMEMWB.control.func_bits = 0;
  rMEMWB.control.jal = 0;
  rMEMWB.control.read_data(rMEMWB.control.instruction_control_map, "data.txt");
  uint32_t dummy = 0;
  while (!end) {
      num_cycles++;
      stallhaz = false;
      rIDEX.stall = false;
      rIFID.stall = false;
      if ((reg_file.pc) == end_pc+16)
      {
          end = true;
      }
      // forwarding happens here needs cleaning

       if (rIDEX.rs_num == rMEMWB.r_write)
       {
         if (rMEMWB.opcode == 0x5 || rMEMWB.opcode == 0x4 || rMEMWB.opcode == 0x3 || rMEMWB.opcode == 0x2 || rMEMWB.opcode == 0x28 || rMEMWB.opcode == 0x29 || rMEMWB.opcode == 0x38 || rMEMWB.opcode == 0x2b || (rMEMWB.opcode == 0x0 && rMEMWB.funct_bits == 0x8))
         {
           noFmemwb = true;
         }else
         {
           noFmemwb = false;
         }
         if (!rIDEX.control.shift)
         {
           if (1/*rMEMWB.valid*/)
           {
             if (!noFmemwb)
             {
               if (rMEMWB.control.mem_to_reg)
               {
                 rIDEX.read_data_1 = rMEMWB.read_data;
               }else
               {
                 rIDEX.read_data_1 = rMEMWB.alu_result;
               }
             }
           }
         }
       }
       if (rIDEX.rt_num == rMEMWB.r_write)
       {
         if (rMEMWB.opcode == 0x5 || rMEMWB.opcode == 0x4 || rMEMWB.opcode == 0x3 || rMEMWB.opcode == 0x2 || rMEMWB.opcode == 0x28 || rMEMWB.opcode == 0x29 || rMEMWB.opcode == 0x38 || rMEMWB.opcode == 0x2b || (rMEMWB.opcode == 0x0 && rMEMWB.funct_bits == 0x8))
         {
           noFmemwb = true;
         }else
         {
           noFmemwb = false;
         }
         if (1/*rMEMWB.valid*/)
         {
           if (!noFmemwb)
           {
             if (rMEMWB.control.mem_to_reg)
             {
               rIDEX.read_data_2 = rMEMWB.read_data;
             }else
             {
               rIDEX.read_data_2 = rMEMWB.alu_result;
             }
           }
         }
       }

       if (rIDEX.rs_num == rEXMEM.r_write)
       {
         if (rEXMEM.opcode == 0x5 || rEXMEM.opcode == 0x4 || rEXMEM.opcode == 0x3 || rEXMEM.opcode == 0x2 || rEXMEM.opcode == 0x28 || rEXMEM.opcode == 0x29 || rEXMEM.opcode == 0x38 || rEXMEM.opcode == 0x2b || (rEXMEM.opcode == 0x0 && rEXMEM.funct_bits == 0x8))
         {
           noFexmem = true;
         }else
         {
           noFexmem = false;
         }
         if (!rIDEX.control.shift)
         {
           if (1/*rEXMEM.valid*/)
           {
             if (!noFexmem)
             {
               if (rEXMEM.opcode == 0x24 || rEXMEM.opcode == 0x25 || rEXMEM.opcode == 0x30 || rEXMEM.opcode == 0xf || rEXMEM.opcode == 0x23)
               {
                 rIDEX.stall = true;
                 rIFID.stall = true;
                 stallhaz = true;
               }else
               {
                 rIDEX.read_data_1 = rEXMEM.alu_result;
               }
             }
           }
         }
       }
       if (rIDEX.rt_num == rEXMEM.r_write)
       {
         if (rEXMEM.opcode == 0x5 || rEXMEM.opcode == 0x4 || rEXMEM.opcode == 0x3 || rEXMEM.opcode == 0x2 || rEXMEM.opcode == 0x28 || rEXMEM.opcode == 0x29 || rEXMEM.opcode == 0x38 || rEXMEM.opcode == 0x2b || (rEXMEM.opcode == 0x0 && rEXMEM.funct_bits == 0x8))
         {
           noFexmem = true;
         }else
         {
           noFexmem = false;
         }
           if (1/*rEXMEM.valid*/)
           {
             if (!noFexmem)
             {
               if (rEXMEM.opcode == 0x24 || rEXMEM.opcode == 0x25 || rEXMEM.opcode == 0x30 || rEXMEM.opcode == 0xf || rEXMEM.opcode == 0x23)
               {
                 rIDEX.stall = true;
                 rIFID.stall = true;
                 stallhaz = true;
               }else
               {
                 rIDEX.read_data_2 = rEXMEM.alu_result;
               }
             }
           }
       }

      //This is the write back stage of things
      if (rMEMWB.valid)
      {
        num_instrs++;
      }

      //~~~~~~~~~~~~~~~~~~~~~LOAD HALF WORDS AND BYTE MASKING~~~~~~~~~~~~~~~~~~~~~
      if(rMEMWB.opcode == 37) // load half unsigned
      {
        rMEMWB.read_data &= 0x0000ffff;
      }
      if(rMEMWB.opcode == 36) // load byte i
      {
        rMEMWB.read_data &= 0x000000ff;
      }
      if (false/*rMEMWB.control.jal*/)
      {
        //  reg_file.access(dummy, dummy, dummy, dummy, 0x1f, true, rMEMWB.jal_reg);
      }else if (rMEMWB.control.mem_to_reg)
      {
          reg_file.access(dummy, dummy, dummy, dummy, rMEMWB.r_write, rMEMWB.control.reg_write, rMEMWB.read_data);
      }else
      {
        if (!rMEMWB.control.mem_write)
        {
          reg_file.access(dummy, dummy, dummy, dummy, rMEMWB.r_write, rMEMWB.control.reg_write, rMEMWB.alu_result);
        }
      }
    //  if(rMEMWB.opcode == 37) // load half unsigned
      //{
          //this var need to be read_data mem_data &= 0x0000ffff;
    //  }
      //if(rMEMWB.opcode == 36) // load byte i
    //  {
          //this var need to be read_data mem_data &= 0x000000ff;
    //  }

    // This is the mem stage of things
      if(!rMEMWB.stall)
      {
          //~~~~~~~~~~~~~~~~~~~~~~~~PASS VAULES: EXME->MEMWB~~~~~~~~~~~~~~~~~~~~~~~~
          rMEMWB.control = rEXMEM.control;
          rMEMWB.alu_result = rEXMEM.alu_result;
          rMEMWB.r_write = rEXMEM.r_write;
          rMEMWB.jal_reg = rEXMEM.pc_adder + 4;
          rMEMWB.opcode = rEXMEM.opcode;
          rMEMWB.valid = rEXMEM.valid;
          rMEMWB.pc_adder = rEXMEM.pc_adder;
          rMEMWB.funct_bits = rEXMEM.funct_bits;
          //rMEMWB.read_data = 1; //gonna need some help here: not need value set by being passed by reference
          //~~~~~~~~~~~~~~~~~~~~~STORE HALF WOES AND BYTE LOGIC~~~~~~~~~~~~~~~~~~~~~
          if(rMEMWB.opcode == 0x29) // store half word jank
          {
              memory.access(rEXMEM.alu_result*4+400, bogus, rEXMEM.read_data_2, 1, 0);
              rEXMEM.read_data_2 = (rEXMEM.read_data_2 & 0x0000ffff) | (bogus & 0xffff0000);
              memory.access(rEXMEM.alu_result*4+400, rMEMWB.read_data, rEXMEM.read_data_2, 0, 1);
          }else if(rMEMWB.opcode == 0x28) // store half byte jank
          {
              memory.access(rEXMEM.alu_result*4+400, bogus, rEXMEM.read_data_2, 1, 0);
              rEXMEM.read_data_2 = (rEXMEM.read_data_2 & 0x000000ff) | (bogus & 0xffffff00);
              memory.access(rEXMEM.alu_result*4+400, rMEMWB.read_data, rEXMEM.read_data_2, 0, 1);
          }else
          {
              memory.access( rEXMEM.alu_result*4+400, rMEMWB.read_data, rEXMEM.read_data_2, rEXMEM.control.mem_read, rEXMEM.control.mem_write);
          }
      }


      // This is the EX stage of things
      if(!rEXMEM.stall)
      {
        if (!stallhaz)
        {
          rEXMEM.valid = rIDEX.valid;
          rEXMEM.last = rIDEX.last;
          rEXMEM.control = rIDEX.control;
          rEXMEM.pc_alu_result = rIDEX.pc + rIDEX.sign_extended*4;
          rEXMEM.pc_adder = rIDEX.pc;
          rEXMEM.read_data_2 = rIDEX.read_data_2;
          rEXMEM.r_write = rIDEX.r_write;
          rEXMEM.sign_extended = rIDEX.sign_extended;
          rEXMEM.funct_bits = rIDEX.funct_bits;
          rEXMEM.jump_pc = rIDEX.jump_pc;
          rEXMEM.opcode = rIDEX.opcode;
          alu.generate_control_inputs(rIDEX.control.ALU_op, rIDEX.funct_bits, rIDEX.opcode);
          if (!rIDEX.control.ALU_src)
          {
            rEXMEM.alu_result = alu.execute(rIDEX.read_data_1, rIDEX.read_data_2, rEXMEM.alu_zero);
          }
          else
          {
            rEXMEM.alu_result = alu.execute(rIDEX.read_data_1, rIDEX.sign_extended, rEXMEM.alu_zero);

          }
        }else
        {
          rEXMEM.valid = false;
          rEXMEM.control.decode(0);
          rEXMEM.pc_alu_result = 0;
          rEXMEM.pc_adder = 0;
          rEXMEM.read_data_2 = 0;
          rEXMEM.r_write = 0;
          rEXMEM.sign_extended = 0;
          rEXMEM.funct_bits = 0;
          rEXMEM.jump_pc = 0;
          rEXMEM.opcode = 0;
          alu.generate_control_inputs(rEXMEM.control.ALU_op, 0, 0);
          rEXMEM.alu_result = 0;
          //cout << "I helped";
        }
      }


      // This is the ID stage of things
      if(!rIDEX.stall)
      {
          //~~~~~~~~~~~~~~~DECODING CONTROL BITS~~~~~~~~~~~~~~~~~~~~
          rIDEX.control.decode(rIFID.instruction); // decode control bits
          //~~~~~~~~~~~~~~~~PASS VALUES: IFID->IDEX~~~~~~~~~~~~~~~~
          rIDEX.pc = rIFID.pc;
          rIDEX.valid = rIFID.valid;
          rIDEX.jump_pc = rIFID.jump_pc;
          //rIDEX.read_data_2 = ((rIFID.instruction>>16) & 31);
          rIDEX.sign_extended = (short)(rIFID.instruction & 0x0000ffff);
          if (!rIDEX.control.reg_dest)
          {
            rIDEX.r_write = ((rIFID.instruction>>16) & 31);
          }else
          {
            rIDEX.r_write = ((rIFID.instruction>>11) & 31);
          }
          rIDEX.funct_bits = (rIFID.instruction & 63);
          rIDEX.rs_num = ((rIFID.instruction>>0x15) & 0x1F);
          rIDEX.rt_num = ((rIFID.instruction>>0x10) & 0x1F); //[25-11]
          rIDEX.rd_num = ((rIFID.instruction>>0xB) & 0x1F);
          rIDEX.opcode = ((rIFID.instruction>>26) & 63);

      //    rIDEX.shamt = ((rIFID.instruction>>0x6) & 0x1F); no longer needed
          //~~~~~~~~~~~~~~~REG FILE ACCESS~~~~~~~~~~~~~~~~~~~~~~~~~~
          reg_file.access(rIDEX.rs_num, rIDEX.rt_num, rIDEX.read_data_1, rIDEX.read_data_2, MUX(rIDEX.control.reg_dest, rIDEX.rd_num, rIDEX.rt_num), 0, 0);
          if (rIDEX.control.shift) // if this is a shift, the alu needs the shamt instead of rs
          {
            rIDEX.read_data_1 = ((rIFID.instruction>>0x6) & 0x1F);
          }
          if(rIDEX.control.jump)
          {
            switch(rIDEX.opcode)
            {
                case 0:
                    reg_file.pc = rIDEX.read_data_1<<2;
                    break;
                case 2:
                    reg_file.pc = rIDEX.jump_pc<<2;
                    break;
                case 3:
                    //reg_file.access(0, 0, reg_data_1, reg_data_2, 31, true, (reg_file.pc+8));
                    reg_file.pc = rIDEX.jump_pc<<2;
                    break;
            }
          }
        if (rIDEX.opcode == 4)
        {
          rIDEX.last = bpu.predict(rIDEX.pc);
          if (rIDEX.last)
          {
            reg_file.pc = (rIFID.instruction & 0x0000ffff);
          }
        }
        if (rIDEX.opcode == 5)
        {
          rIDEX.last = bpu.predict(rIDEX.pc);
          if (rIDEX.last)
          {
            reg_file.pc = (rIFID.instruction & 0x0000ffff);
          }
        }
      }
      if (rEXMEM.opcode == 4) // janky branch equal
      {
          if (rEXMEM.alu_zero)
          {
            bpu.update(rEXMEM.pc_adder, true);
          }else
          {
            bpu.update(rEXMEM.pc_adder, false);
          }
          if (rEXMEM.last && !rEXMEM.alu_zero)
          {
            reg_file.pc = rEXMEM.pc_adder;
            rIFID.valid = false;
            rIFID.instruction = 0;
            rIDEX.stall = true;
            rIFID.stall = true;
          }else if (!rEXMEM.last && rEXMEM.alu_zero)
          {
            reg_file.pc = rEXMEM.pc_alu_result;
            rIFID.valid = false;
            rIFID.instruction = 0;
            rIDEX.stall = true;
            rIFID.stall = true;
          }
      }
      // correction time
      if (rEXMEM.opcode == 5) // janky branch not equal
      {
        if (rEXMEM.alu_zero)
        {
          bpu.update(rEXMEM.pc_adder, false);
        }else
        {
          bpu.update(rEXMEM.pc_adder, true);
        }
        if (!rEXMEM.last && !rEXMEM.alu_zero)
        {
          reg_file.pc = rEXMEM.pc_alu_result;
          rIFID.valid = false;
          rIFID.instruction = 0;
          rIDEX.stall = true;
          rIFID.stall = true;
        }else if (rEXMEM.last && rEXMEM.alu_zero)
        {
          reg_file.pc = rEXMEM.pc_adder;
          rIFID.valid = false;
          rIFID.instruction = 0;
          rIDEX.stall = true;
          rIFID.stall = true;
        }
      }

      // This is the If stage of things
      if(!rIFID.stall)
      {
        if (rIFID.instruction != 0)
        {
          rIFID.valid = true;
        }else
        {
          rIFID.valid = false;
        }
        memory.access(reg_file.pc, rIFID.instruction, 0, 1, 0);
        reg_file.pc += 4;
        rIFID.pc = reg_file.pc;
        rIFID.jump_pc = (rIFID.instruction & 67108863);
      }




      cout << "CYCLE" << num_cycles << "\n";

      reg_file.print(); // used for automated testing
      //num_instrs += committed_insts;
  }

  cout << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";}

void io_superscalar_main_loop(Registers &reg_file, Memory &memory, uint32_t end_pc) {
  ALU alu;
  bool fileDebug = true;
  bool end = false;
  bool stallhaz = false;
  bool noFmemwb = false;
  bool noFexmem = false;
  uint32_t num_cycles = 0;
  uint32_t num_instrs = 0;
  uint32_t bogus = 0;
  struct IFID rIFID1;
  struct IFID rIFID2;
  rIFID1.stall = false;
  rIFID2.stall = false;
  rIFID1.valid = false;
  rIFID2.valid = false;
  rIFID1.pc = 0;
  rIFID2.pc = 0;
  rIFID1.jump_pc = 0;
  rIFID2.jump_pc = 0;
  struct IDEX rIDEX1;
  struct IDEX rIDEX2;
  rIDEX1.stall = false;
  rIDEX1.valid = false;
  rIDEX2.stall = false;
  rIDEX2.valid = false;
  //Init control
  rIDEX1.control.reg_dest = 0;
  rIDEX1.control.jump = 0;
  rIDEX1.control.branchne = 0;
  rIDEX1.control.branch = 0;
  rIDEX1.control.mem_read = 0;
  rIDEX1.control.mem_to_reg = 0;
  rIDEX1.control.ALU_op = 0;
  rIDEX1.control.mem_write = 0;
  rIDEX1.control.ALU_src = 0;
  rIDEX1.control.reg_write = 0;
  rIDEX1.control.opcode = 0;
  rIDEX1.control.shift = 0;
  rIDEX1.control.func_bits = 0;
  rIDEX1.control.jal = 0;
  rIDEX1.control.read_data(rIDEX1.control.instruction_control_map, "data.txt"); // import control bit mapping
  rIDEX2.control.reg_dest = 0;
  rIDEX2.control.jump = 0;
  rIDEX2.control.branchne = 0;
  rIDEX2.control.branch = 0;
  rIDEX2.control.mem_read = 0;
  rIDEX2.control.mem_to_reg = 0;
  rIDEX2.control.ALU_op = 0;
  rIDEX2.control.mem_write = 0;
  rIDEX2.control.ALU_src = 0;
  rIDEX2.control.reg_write = 0;
  rIDEX2.control.opcode = 0;
  rIDEX2.control.shift = 0;
  rIDEX2.control.func_bits = 0;
  rIDEX2.control.jal = 0;
  rIDEX2.control.read_data(rIDEX2.control.instruction_control_map, "data.txt");
  struct EXMEM rEXMEM1;
  struct EXMEM rEXMEM2;
  rEXMEM1.stall = false;
  rEXMEM1.valid = false;
  rEXMEM2.stall = false;
  rEXMEM2.valid = false;
  // init control
  rEXMEM1.control.reg_dest = 0;
  rEXMEM1.control.jump = 0;
  rEXMEM1.control.branchne = 0;
  rEXMEM1.control.branch = 0;
  rEXMEM1.control.mem_read = 0;
  rEXMEM1.control.mem_to_reg = 0;
  rEXMEM1.control.ALU_op = 0;
  rEXMEM1.control.mem_write = 0;
  rEXMEM1.control.ALU_src = 0;
  rEXMEM1.control.reg_write = 0;
  rEXMEM1.control.opcode = 0;
  rEXMEM1.control.shift = 0;
  rEXMEM1.control.func_bits = 0;
  rEXMEM1.control.jal = 0;
  rEXMEM1.control.read_data(rEXMEM1.control.instruction_control_map, "data.txt");
  rEXMEM2.control.reg_dest = 0;
  rEXMEM2.control.jump = 0;
  rEXMEM2.control.branchne = 0;
  rEXMEM2.control.branch = 0;
  rEXMEM2.control.mem_read = 0;
  rEXMEM2.control.mem_to_reg = 0;
  rEXMEM2.control.ALU_op = 0;
  rEXMEM2.control.mem_write = 0;
  rEXMEM2.control.ALU_src = 0;
  rEXMEM2.control.reg_write = 0;
  rEXMEM2.control.opcode = 0;
  rEXMEM2.control.shift = 0;
  rEXMEM2.control.func_bits = 0;
  rEXMEM2.control.jal = 0;
  rEXMEM2.control.read_data(rEXMEM2.control.instruction_control_map, "data.txt");
  struct MEMWB rMEMWB1;
  struct MEMWB rMEMWB2;
  rMEMWB1.stall = false;
  rMEMWB1.valid = false;
  rMEMWB2.stall = false;
  rMEMWB2.valid = false;
  // init control
  rMEMWB1.control.reg_dest = 0;
  rMEMWB1.control.jump = 0;
  rMEMWB1.control.branchne = 0;
  rMEMWB1.control.branch = 0;
  rMEMWB1.control.mem_read = 0;
  rMEMWB1.control.mem_to_reg = 0;
  rMEMWB1.control.ALU_op = 0;
  rMEMWB1.control.mem_write = 0;
  rMEMWB1.control.ALU_src = 0;
  rMEMWB1.control.reg_write = 0;
  rMEMWB1.control.opcode = 0;
  rMEMWB1.control.shift = 0;
  rMEMWB1.control.func_bits = 0;
  rMEMWB1.control.jal = 0;
  rMEMWB1.control.read_data(rMEMWB1.control.instruction_control_map, "data.txt");
  rMEMWB2.control.reg_dest = 0;
  rMEMWB2.control.jump = 0;
  rMEMWB2.control.branchne = 0;
  rMEMWB2.control.branch = 0;
  rMEMWB2.control.mem_read = 0;
  rMEMWB2.control.mem_to_reg = 0;
  rMEMWB2.control.ALU_op = 0;
  rMEMWB2.control.mem_write = 0;
  rMEMWB2.control.ALU_src = 0;
  rMEMWB2.control.reg_write = 0;
  rMEMWB2.control.opcode = 0;
  rMEMWB2.control.shift = 0;
  rMEMWB2.control.func_bits = 0;
  rMEMWB2.control.jal = 0;
  rMEMWB2.control.read_data(rMEMWB2.control.instruction_control_map, "data.txt");
  uint32_t dummy = 0;
  while (!end) {
      num_cycles++;
      stallhaz = false;
      rIDEX1.stall = false;
      rIFID1.stall = false;
      rIDEX2.stall = false;
      rIFID2.stall = false;
      if ((reg_file.pc) == end_pc+20)
      {
          end = true;
      }
      // forwarding happens here needs cleaning
/*
       if (rIDEX.rs_num == rMEMWB.r_write)
       {
         if (rMEMWB.opcode == 0x5 || rMEMWB.opcode == 0x4 || rMEMWB.opcode == 0x3 || rMEMWB.opcode == 0x2 || rMEMWB.opcode == 0x28 || rMEMWB.opcode == 0x29 || rMEMWB.opcode == 0x38 || rMEMWB.opcode == 0x2b || (rMEMWB.opcode == 0x0 && rMEMWB.funct_bits == 0x8))
         {
           noFmemwb = true;
         }else
         {
           noFmemwb = false;
         }
         if (!rIDEX.control.shift)
         {
           if (1)
           {
             if (!noFmemwb)
             {
               if (rMEMWB.control.mem_to_reg)
               {
                 rIDEX.read_data_1 = rMEMWB.read_data;
               }else
               {
                 rIDEX.read_data_1 = rMEMWB.alu_result;
               }
             }
           }
         }
       }
       if (rIDEX.rt_num == rMEMWB.r_write)
       {
         if (rMEMWB.opcode == 0x5 || rMEMWB.opcode == 0x4 || rMEMWB.opcode == 0x3 || rMEMWB.opcode == 0x2 || rMEMWB.opcode == 0x28 || rMEMWB.opcode == 0x29 || rMEMWB.opcode == 0x38 || rMEMWB.opcode == 0x2b || (rMEMWB.opcode == 0x0 && rMEMWB.funct_bits == 0x8))
         {
           noFmemwb = true;
         }else
         {
           noFmemwb = false;
         }
         if (1)
         {
           if (!noFmemwb)
           {
             if (rMEMWB.control.mem_to_reg)
             {
               rIDEX.read_data_2 = rMEMWB.read_data;
             }else
             {
               rIDEX.read_data_2 = rMEMWB.alu_result;
             }
           }
         }
       }

       if (rIDEX.rs_num == rEXMEM.r_write)
       {
         if (rEXMEM.opcode == 0x5 || rEXMEM.opcode == 0x4 || rEXMEM.opcode == 0x3 || rEXMEM.opcode == 0x2 || rEXMEM.opcode == 0x28 || rEXMEM.opcode == 0x29 || rEXMEM.opcode == 0x38 || rEXMEM.opcode == 0x2b || (rEXMEM.opcode == 0x0 && rEXMEM.funct_bits == 0x8))
         {
           noFexmem = true;
         }else
         {
           noFexmem = false;
         }
         if (!rIDEX.control.shift)
         {
           if (1)
           {
             if (!noFexmem)
             {
               if (rEXMEM.opcode == 0x24 || rEXMEM.opcode == 0x25 || rEXMEM.opcode == 0x30 || rEXMEM.opcode == 0xf || rEXMEM.opcode == 0x23)
               {
                 rIDEX.stall = true;
                 rIFID.stall = true;
                 stallhaz = true;
               }else
               {
                 rIDEX.read_data_1 = rEXMEM.alu_result;
               }
             }
           }
         }
       }
       if (rIDEX.rt_num == rEXMEM.r_write)
       {
         if (rEXMEM.opcode == 0x5 || rEXMEM.opcode == 0x4 || rEXMEM.opcode == 0x3 || rEXMEM.opcode == 0x2 || rEXMEM.opcode == 0x28 || rEXMEM.opcode == 0x29 || rEXMEM.opcode == 0x38 || rEXMEM.opcode == 0x2b || (rEXMEM.opcode == 0x0 && rEXMEM.funct_bits == 0x8))
         {
           noFexmem = true;
         }else
         {
           noFexmem = false;
         }
           if (1)
           {
             if (!noFexmem)
             {
               if (rEXMEM.opcode == 0x24 || rEXMEM.opcode == 0x25 || rEXMEM.opcode == 0x30 || rEXMEM.opcode == 0xf || rEXMEM.opcode == 0x23)
               {
                 rIDEX.stall = true;
                 rIFID.stall = true;
                 stallhaz = true;
               }else
               {
                 rIDEX.read_data_2 = rEXMEM.alu_result;
               }
             }
           }
       }
       */
      //This is the write back stage of things
      if (rMEMWB1.valid)
      {
        num_instrs++;
      }
      if (rMEMWB2.valid)
      {
        num_instrs++;
      }

      //~~~~~~~~~~~~~~~~~~~~~LOAD HALF WORDS AND BYTE MASKING~~~~~~~~~~~~~~~~~~~~~
      if(rMEMWB1.opcode == 37) // load half unsigned
      {
        rMEMWB1.read_data &= 0x0000ffff;
      }
      if(rMEMWB1.opcode == 36) // load byte i
      {
        rMEMWB1.read_data &= 0x000000ff;
      }
      if (false/*rMEMWB.control.jal*/)
      {
        //  reg_file.access(dummy, dummy, dummy, dummy, 0x1f, true, rMEMWB.jal_reg);
      }else if (rMEMWB1.control.mem_to_reg)
      {
          reg_file.access(dummy, dummy, dummy, dummy, rMEMWB1.r_write, rMEMWB1.control.reg_write, rMEMWB1.read_data);
      }else
      {
        if (!rMEMWB1.control.mem_write)
        {
          reg_file.access(dummy, dummy, dummy, dummy, rMEMWB1.r_write, rMEMWB1.control.reg_write, rMEMWB1.alu_result);
        }
      }

      if(rMEMWB2.opcode == 37) // load half unsigned
      {
        rMEMWB2.read_data &= 0x0000ffff;
      }
      if(rMEMWB2.opcode == 36) // load byte i
      {
        rMEMWB2.read_data &= 0x000000ff;
      }
      if (false/*rMEMWB.control.jal*/)
      {
        //  reg_file.access(dummy, dummy, dummy, dummy, 0x1f, true, rMEMWB.jal_reg);
      }else if (rMEMWB2.control.mem_to_reg)
      {
          reg_file.access(dummy, dummy, dummy, dummy, rMEMWB2.r_write, rMEMWB2.control.reg_write, rMEMWB2.read_data);
      }else
      {
        if (!rMEMWB2.control.mem_write)
        {
          reg_file.access(dummy, dummy, dummy, dummy, rMEMWB2.r_write, rMEMWB2.control.reg_write, rMEMWB2.alu_result);
        }
      }
    //  if(rMEMWB.opcode == 37) // load half unsigned
      //{
          //this var need to be read_data mem_data &= 0x0000ffff;
    //  }
      //if(rMEMWB.opcode == 36) // load byte i
    //  {
          //this var need to be read_data mem_data &= 0x000000ff;
    //  }

    // This is the mem stage of things
      if(!rMEMWB1.stall)
      {
          //~~~~~~~~~~~~~~~~~~~~~~~~PASS VAULES: EXME->MEMWB~~~~~~~~~~~~~~~~~~~~~~~~
          rMEMWB1.control = rEXMEM1.control;
          rMEMWB1.alu_result = rEXMEM1.alu_result;
          rMEMWB1.r_write = rEXMEM1.r_write;
          rMEMWB1.jal_reg = rEXMEM1.pc_adder + 4;
          rMEMWB1.opcode = rEXMEM1.opcode;
          rMEMWB1.valid = rEXMEM1.valid;
          rMEMWB1.pc_adder = rEXMEM1.pc_adder;
          rMEMWB1.funct_bits = rEXMEM1.funct_bits;
          //rMEMWB.read_data = 1; //gonna need some help here: not need value set by being passed by reference
          //~~~~~~~~~~~~~~~~~~~~~STORE HALF WOES AND BYTE LOGIC~~~~~~~~~~~~~~~~~~~~~
          if(rMEMWB1.opcode == 0x29) // store half word jank
          {
              memory.access(rEXMEM1.alu_result*4+400, bogus, rEXMEM1.read_data_2, 1, 0);
              rEXMEM1.read_data_2 = (rEXMEM1.read_data_2 & 0x0000ffff) | (bogus & 0xffff0000);
              memory.access(rEXMEM1.alu_result*4+400, rMEMWB1.read_data, rEXMEM1.read_data_2, 0, 1);
          }else if(rMEMWB1.opcode == 0x28) // store half byte jank
          {
              memory.access(rEXMEM1.alu_result*4+400, bogus, rEXMEM1.read_data_2, 1, 0);
              rEXMEM1.read_data_2 = (rEXMEM1.read_data_2 & 0x000000ff) | (bogus & 0xffffff00);
              memory.access(rEXMEM1.alu_result*4+400, rMEMWB1.read_data, rEXMEM1.read_data_2, 0, 1);
          }else
          {
              memory.access( rEXMEM1.alu_result*4+400, rMEMWB1.read_data, rEXMEM1.read_data_2, rEXMEM1.control.mem_read, rEXMEM1.control.mem_write);
          }
      }

      if(!rMEMWB2.stall)
      {
          //~~~~~~~~~~~~~~~~~~~~~~~~PASS VAULES: EXME->MEMWB~~~~~~~~~~~~~~~~~~~~~~~~
          rMEMWB2.control = rEXMEM2.control;
          rMEMWB2.alu_result = rEXMEM2.alu_result;
          rMEMWB2.r_write = rEXMEM2.r_write;
          rMEMWB2.jal_reg = rEXMEM2.pc_adder + 4;
          rMEMWB2.opcode = rEXMEM2.opcode;
          rMEMWB2.valid = rEXMEM2.valid;
          rMEMWB2.pc_adder = rEXMEM2.pc_adder;
          rMEMWB2.funct_bits = rEXMEM2.funct_bits;
          //rMEMWB.read_data = 1; //gonna need some help here: not need value set by being passed by reference
          //~~~~~~~~~~~~~~~~~~~~~STORE HALF WOES AND BYTE LOGIC~~~~~~~~~~~~~~~~~~~~~
          if(rMEMWB2.opcode == 0x29) // store half word jank
          {
              memory.access(rEXMEM2.alu_result*4+400, bogus, rEXMEM2.read_data_2, 1, 0);
              rEXMEM2.read_data_2 = (rEXMEM2.read_data_2 & 0x0000ffff) | (bogus & 0xffff0000);
              memory.access(rEXMEM2.alu_result*4+400, rMEMWB2.read_data, rEXMEM2.read_data_2, 0, 1);
          }else if(rMEMWB2.opcode == 0x28) // store half byte jank
          {
              memory.access(rEXMEM2.alu_result*4+400, bogus, rEXMEM2.read_data_2, 1, 0);
              rEXMEM2.read_data_2 = (rEXMEM2.read_data_2 & 0x000000ff) | (bogus & 0xffffff00);
              memory.access(rEXMEM2.alu_result*4+400, rMEMWB2.read_data, rEXMEM2.read_data_2, 0, 1);
          }else
          {
              memory.access( rEXMEM2.alu_result*4+400, rMEMWB2.read_data, rEXMEM2.read_data_2, rEXMEM2.control.mem_read, rEXMEM2.control.mem_write);
          }
      }

      // THis is the ex stage
      if(!rEXMEM1.stall)
      {
        if (!stallhaz)
        {
          rEXMEM1.valid = rIDEX1.valid;
          rEXMEM1.control = rIDEX1.control;
          rEXMEM1.pc_alu_result = rIDEX1.pc + rIDEX1.sign_extended*4;
          rEXMEM1.pc_adder = rIDEX1.pc;
          rEXMEM1.read_data_2 = rIDEX1.read_data_2;
          rEXMEM1.r_write = rIDEX1.r_write;
          rEXMEM1.sign_extended = rIDEX1.sign_extended;
          rEXMEM1.funct_bits = rIDEX1.funct_bits;
          rEXMEM1.jump_pc = rIDEX1.jump_pc;
          rEXMEM1.opcode = rIDEX1.opcode;
          alu.generate_control_inputs(rIDEX1.control.ALU_op, rIDEX1.funct_bits, rIDEX1.opcode);
          if (!rIDEX1.control.ALU_src)
          {
            rEXMEM1.alu_result = alu.execute(rIDEX1.read_data_1, rIDEX1.read_data_2, rEXMEM1.alu_zero);
          }
          else
          {
            rEXMEM1.alu_result = alu.execute(rIDEX1.read_data_1, rIDEX1.sign_extended, rEXMEM1.alu_zero);

          }
        }else
        {
          rEXMEM1.valid = false;
          rEXMEM1.control.decode(0);
          rEXMEM1.pc_alu_result = 0;
          rEXMEM1.pc_adder = 0;
          rEXMEM1.read_data_2 = 0;
          rEXMEM1.r_write = 0;
          rEXMEM1.sign_extended = 0;
          rEXMEM1.funct_bits = 0;
          rEXMEM1.jump_pc = 0;
          rEXMEM1.opcode = 0;
          alu.generate_control_inputs(rEXMEM1.control.ALU_op, 0, 0);
          rEXMEM1.alu_result = 0;
          //cout << "I helped";
        }
      }
      if(!rEXMEM2.stall)
      {
        if (!stallhaz)
        {
          rEXMEM2.valid = rIDEX2.valid;
          rEXMEM2.control = rIDEX2.control;
          rEXMEM2.pc_alu_result = rIDEX2.pc + rIDEX2.sign_extended*4;
          rEXMEM2.pc_adder = rIDEX2.pc;
          rEXMEM2.read_data_2 = rIDEX2.read_data_2;
          rEXMEM2.r_write = rIDEX2.r_write;
          rEXMEM2.sign_extended = rIDEX2.sign_extended;
          rEXMEM2.funct_bits = rIDEX2.funct_bits;
          rEXMEM2.jump_pc = rIDEX2.jump_pc;
          rEXMEM2.opcode = rIDEX2.opcode;
          alu.generate_control_inputs(rIDEX2.control.ALU_op, rIDEX2.funct_bits, rIDEX2.opcode);
          if (!rIDEX2.control.ALU_src)
          {
            rEXMEM2.alu_result = alu.execute(rIDEX2.read_data_1, rIDEX2.read_data_2, rEXMEM2.alu_zero);
          }
          else
          {
            rEXMEM2.alu_result = alu.execute(rIDEX2.read_data_1, rIDEX2.sign_extended, rEXMEM2.alu_zero);

          }
        }else
        {
          rEXMEM2.valid = false;
          rEXMEM2.control.decode(0);
          rEXMEM2.pc_alu_result = 0;
          rEXMEM2.pc_adder = 0;
          rEXMEM2.read_data_2 = 0;
          rEXMEM2.r_write = 0;
          rEXMEM2.sign_extended = 0;
          rEXMEM2.funct_bits = 0;
          rEXMEM2.jump_pc = 0;
          rEXMEM2.opcode = 0;
          alu.generate_control_inputs(rEXMEM2.control.ALU_op, 0, 0);
          rEXMEM2.alu_result = 0;
          //cout << "I helped";
        }
      }

      // This is the ID stage of things
      if(!rIDEX1.stall)
      {
          //~~~~~~~~~~~~~~~DECODING CONTROL BITS~~~~~~~~~~~~~~~~~~~~
          rIDEX1.control.decode(rIFID1.instruction); // decode control bits
          //~~~~~~~~~~~~~~~~PASS VALUES: IFID->IDEX~~~~~~~~~~~~~~~~
          rIDEX1.pc = rIFID1.pc;
          rIDEX1.valid = rIFID1.valid;
          rIDEX1.jump_pc = rIFID1.jump_pc;
          //rIDEX.read_data_2 = ((rIFID.instruction>>16) & 31);
          rIDEX1.sign_extended = (short)(rIFID1.instruction & 0x0000ffff);
          if (!rIDEX1.control.reg_dest)
          {
            rIDEX1.r_write = ((rIFID1.instruction>>16) & 31);
          }else
          {
            rIDEX1.r_write = ((rIFID1.instruction>>11) & 31);
          }
          rIDEX1.funct_bits = (rIFID1.instruction & 63);
          rIDEX1.rs_num = ((rIFID1.instruction>>0x15) & 0x1F);
          rIDEX1.rt_num = ((rIFID1.instruction>>0x10) & 0x1F); //[25-11]
          rIDEX1.rd_num = ((rIFID1.instruction>>0xB) & 0x1F);
          rIDEX1.opcode = ((rIFID1.instruction>>26) & 63);

      //    rIDEX.shamt = ((rIFID.instruction>>0x6) & 0x1F); no longer needed
          //~~~~~~~~~~~~~~~REG FILE ACCESS~~~~~~~~~~~~~~~~~~~~~~~~~~
          reg_file.access(rIDEX1.rs_num, rIDEX1.rt_num, rIDEX1.read_data_1, rIDEX1.read_data_2, MUX(rIDEX1.control.reg_dest, rIDEX1.rd_num, rIDEX1.rt_num), 0, 0);
          if (rIDEX1.control.shift) // if this is a shift, the alu needs the shamt instead of rs
          {
            rIDEX1.read_data_1 = ((rIFID1.instruction>>0x6) & 0x1F);
          }
          if(rIDEX1.control.jump)
          {
            switch(rIDEX1.opcode)
            {
                case 0:
                    reg_file.pc = rIDEX1.read_data_1<<2;
                    break;
                case 2:
                    reg_file.pc = rIDEX1.jump_pc<<2;
                    break;
                case 3:
                    //reg_file.access(0, 0, reg_data_1, reg_data_2, 31, true, (reg_file.pc+8));
                    reg_file.pc = rIDEX1.jump_pc<<2;
                    break;
            }
          }
        if (rEXMEM1.opcode == 4 && rEXMEM1.alu_zero) // janky branch equal
        {
            // reg_file.pc += immediate * 4; not sure
            reg_file.pc = rEXMEM1.pc_alu_result;
            rIFID1.valid = false;
            rIFID1.instruction = 0;
            rIDEX1.stall = true;
            rIFID1.stall = true;
        }
        if (rEXMEM1.opcode == 5 && !rEXMEM1.alu_zero) // janky branch not equal
        {
            //reg_file.pc += immediate * 4; not sure
            reg_file.pc = rEXMEM1.pc_alu_result;
            rIFID1.valid = false;
            rIFID1.instruction = 0;
            rIDEX1.stall = true;
            rIFID1.stall = true;
        }
      }
      if(!rIDEX2.stall)
      {
          //~~~~~~~~~~~~~~~DECODING CONTROL BITS~~~~~~~~~~~~~~~~~~~~
          rIDEX2.control.decode(rIFID2.instruction); // decode control bits
          //~~~~~~~~~~~~~~~~PASS VALUES: IFID->IDEX~~~~~~~~~~~~~~~~
          rIDEX2.pc = rIFID2.pc;
          rIDEX2.valid = rIFID2.valid;
          rIDEX2.jump_pc = rIFID2.jump_pc;
          //rIDEX.read_data_2 = ((rIFID.instruction>>16) & 31);
          rIDEX2.sign_extended = (short)(rIFID2.instruction & 0x0000ffff);
          if (!rIDEX2.control.reg_dest)
          {
            rIDEX2.r_write = ((rIFID2.instruction>>16) & 31);
          }else
          {
            rIDEX2.r_write = ((rIFID2.instruction>>11) & 31);
          }
          rIDEX2.funct_bits = (rIFID2.instruction & 63);
          rIDEX2.rs_num = ((rIFID2.instruction>>0x15) & 0x1F);
          rIDEX2.rt_num = ((rIFID2.instruction>>0x10) & 0x1F); //[25-11]
          rIDEX2.rd_num = ((rIFID2.instruction>>0xB) & 0x1F);
          rIDEX2.opcode = ((rIFID2.instruction>>26) & 63);

      //    rIDEX.shamt = ((rIFID.instruction>>0x6) & 0x1F); no longer needed
          //~~~~~~~~~~~~~~~REG FILE ACCESS~~~~~~~~~~~~~~~~~~~~~~~~~~
          reg_file.access(rIDEX2.rs_num, rIDEX2.rt_num, rIDEX2.read_data_1, rIDEX2.read_data_2, MUX(rIDEX2.control.reg_dest, rIDEX2.rd_num, rIDEX2.rt_num), 0, 0);
          if (rIDEX2.control.shift) // if this is a shift, the alu needs the shamt instead of rs
          {
            rIDEX2.read_data_1 = ((rIFID2.instruction>>0x6) & 0x1F);
          }
          if(rIDEX2.control.jump)
          {
            switch(rIDEX2.opcode)
            {
                case 0:
                    reg_file.pc = rIDEX2.read_data_1<<2;
                    break;
                case 2:
                    reg_file.pc = rIDEX2.jump_pc<<2;
                    break;
                case 3:
                    //reg_file.access(0, 0, reg_data_1, reg_data_2, 31, true, (reg_file.pc+8));
                    reg_file.pc = rIDEX2.jump_pc<<2;
                    break;
            }
          }
        if (rEXMEM2.opcode == 4 && rEXMEM2.alu_zero) // janky branch equal
        {
            // reg_file.pc += immediate * 4; not sure
            reg_file.pc = rEXMEM2.pc_alu_result;
            rIFID2.valid = false;
            rIFID2.instruction = 0;
            rIDEX2.stall = true;
            rIFID2.stall = true;
        }
        if (rEXMEM2.opcode == 5 && !rEXMEM2.alu_zero) // janky branch not equal
        {
            //reg_file.pc += immediate * 4; not sure
            reg_file.pc = rEXMEM2.pc_alu_result;
            rIFID2.valid = false;
            rIFID2.instruction = 0;
            rIDEX2.stall = true;
            rIFID2.stall = true;
        }
      }

      // This is the If stage of things
      if(!rIFID1.stall)
      {
        if (rIFID1.instruction != 0)
        {
          rIFID1.valid = true;
        }else
        {
          rIFID1.valid = false;
        }
        memory.access(reg_file.pc, rIFID1.instruction, 0, 1, 0);
        reg_file.pc += 4;
        rIFID1.pc = reg_file.pc;
        rIFID1.jump_pc = (rIFID1.instruction & 67108863);
      }

      if(!rIFID2.stall)
      {
        if (rIFID2.instruction != 0)
        {
          rIFID2.valid = true;
        }else
        {
          rIFID2.valid = false;
        }
        memory.access(reg_file.pc, rIFID2.instruction, 0, 1, 0);
        reg_file.pc += 4;
        rIFID2.pc = reg_file.pc;
        rIFID2.jump_pc = (rIFID2.instruction & 67108863);
      }




      cout << "CYCLE" << num_cycles << "\n";

      reg_file.print(); // used for automated testing
      //num_instrs += committed_insts;
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
        reg_file.print();
        cout << "\nEND_PC: " << end_pc << endl;
        cout << "\nCOMMITED: " << num_instrs << endl;
        //num_instrs += committed_insts;
        if (1) {
            break;
        }
    }

    cout << "CPI = " << (double)num_cycles/(double)num_instrs << "\n";
}
