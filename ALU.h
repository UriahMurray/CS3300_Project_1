#ifndef ALU_CLASS
#define ALU_CLASS
#include <vector>
#include <cstdint>
#include <iostream>
#include <bitset>

bool DEBUG = true;

class ALU {
    private:
        int ALU_control_inputs;
    public:
        // TODO: add left and right shift codes
        // Generate the control inputs for the ALU
        void generate_control_inputs(int ALU_op, int funct, int opcode) {

            if (ALU_op == 2)
            {
                if (funct == 32 || funct == 33) //decimal to binary 100000, 100001
                {
                    ALU_control_inputs = 2; //this is an add
                }else if (funct == 36) //dec to binary, 100100
                {
                    ALU_control_inputs = 0; //this is an and
                }else if (funct == 39) // dec to binary 100111
                {
                    ALU_control_inputs = 12; // nor 1100
                }else if (funct == 37)  //dec to binary 100101
                {
                    ALU_control_inputs = 1; // 0001 this is an or
                }else if (funct == 42 || funct == 43) //101010, 101011
                {
                    ALU_control_inputs = 7; // 0111 this is set on less than
                }else if (funct == 34 || funct == 35) //100010 100011
                {
                    ALU_control_inputs = 6; // 0110 subtract
                }else if(opcode == 0 && funct == 0)
                {
                    ALU_control_inputs = 8;
                }else if(opcode == 0 && funct == 2)
                {
                    ALU_control_inputs = 9;
                }
            }else if (ALU_op == 1)
            {
                ALU_control_inputs = 6; //branches are 0110 for subtracting the two numbers
            }else if(ALU_op == 0)
            {
                ALU_control_inputs = 2; //store and load are 0010 for adding the address up
            }else // this is for when alu op is 11 aka 3
            {
                if (opcode == 8 || opcode == 9) // dec to 001000, 001001
                {
                    ALU_control_inputs = 2; //this is an add
                }else if (opcode == 12) //001100
                {
                    ALU_control_inputs = 3; //this is an and
                }else if (opcode == 13)  //001101
                {
                    ALU_control_inputs = 1; // 0001 this is an or
                }else if (opcode == 10 || opcode == 11) // 001010, 001011
                {
                    ALU_control_inputs = 7; // 0111 this is set on less than
                } else if(opcode == 15)
                {
                    ALU_control_inputs = 5;
                }
            }

        }
        
        // TODO: add left and right shift
        // execute ALU operations, generate result, and set the zero control signal if necessary
        uint32_t execute(uint32_t operand_1, uint32_t operand_2, uint32_t &ALU_zero)
        {
            uint32_t out;
            if(DEBUG)
            {
                bitset<32> op1(operand_1);
                bitset<32> op2(operand_2);
                cout << "ALU constrol: " << ALU_control_inputs << endl;
                cout << "alu oper1: " << op1 << " " << operand_1 << "\nalu oper2: " << op2 << " " << operand_2 << endl;
            }
            switch(ALU_control_inputs)
            {

                case 0: // AND
                    out = operand_1 & operand_2;
                    ALU_zero = (out == 0) ? 1: 0;
                    cout << "this is working\n";
                    break;
                case 1: // OR
                    out = operand_1 | operand_2;
                    ALU_zero = (out == 0) ? 1: 0;
                    break;
                case 2: // ADD
                    out = operand_1 + operand_2;
                    ALU_zero = (out == 0) ? 1: 0;
                    break;
                case 3: // ANDI (zero) extention
                    out = operand_1 & (operand_2 & 0x0000ffff);
                    break;
                case 4: // not used but or with zero extension
                    out = operand_1 ^ (operand_2 & 0x0000ffff);
                    break;
                case 5:
                    out = operand_2 << 16;
                    break;
                case 6: // SUBTRACT
                    out = operand_1 - operand_2;
                    ALU_zero = (out == 0) ? 1: 0;
                    break;
                case 7: // SET ON LESS THAN
                    out = ((int)operand_1 - (int)operand_2 < 0);
                    break;
                case 8:
                    out = operand_2 << operand_1;
                    ALU_zero = out == 0;
                    break;
                case 9:
                    out = operand_2 >> operand_1;
                    ALU_zero = out == 0;
                    break;
                case 12: //NOR
                    out = ~(operand_1 | operand_2);
                    ALU_zero = (out == 0) ? 1: 0;
            }
            //  cout << "alu out: " << out << endl;
            return out;
        }
};
#endif
