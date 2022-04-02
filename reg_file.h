#ifndef REG_FILE
#define REG_FILE
#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>


using namespace std;
class Registers {
    bool fileDebug = true;
    private:
        std::vector<int32_t> R;
    public:
        uint32_t pc;
        Registers() {
            R.resize(32,0);
        }
	// read_reg_1, read_reg_2 are register numbers from which the data should be read
	// read_data_1, read_data_2 are variables into which the data is read. These are passed by reference
	// write_reg is the register number to which the data needs to be written
	// write is a flag which incidates whether it is a write access or not
	// write_data is the data which needs to be written to write_reg
        void access(int read_reg_1, int read_reg_2, uint32_t &read_data_1, uint32_t &read_data_2, int write_reg, bool write, uint32_t write_data) {
            read_data_1 = R[read_reg_1];
            read_data_2 = R[read_reg_2];
            if(write) {
                R[write_reg] = write_data;
            }
        }
	// Prints the contents of all the registers
        void print() {
            std::ofstream file;
            if(fileDebug)
                file.open("sim_out.txt", std::ios_base::app);

            for(int i = 0; i < 32; ++i) {
                if(fileDebug)
                    file << "R[" << i << "]: " << R[i] << "\n";

                std::cout << "R[" << i << "]: " << R[i] << "\n";
            }
            if(fileDebug)
                file.close();
        }
	// Prints the contents of the register specified by reg 
	// This function should help you debug your code
        void print(int reg) {
            std::cout << "R[" << reg << "]: " << R[reg] << "\n";
        }
            
};
#endif
