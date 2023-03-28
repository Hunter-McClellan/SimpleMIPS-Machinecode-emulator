#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

// The first demo needs to implement the Fetch, Decode, Execute, Mem, and Writeback functions.
// The fist demo does not seem to require that the code fully functions.
// for the first part (demo 1 and 2), we need to implememnt the following functions:
// LW, SW, ADD, SUB, AND, OR, SLT, NOR, BEQ, J

// helper functions
unsigned int isolate_bits(unsigned int input, char start, char len); // returns the isolated field from start to start + len
int sign_extend(short input);

// function predeclaration
void Fetch();
void Decode (unsigned int data);
void Execute (int read_data_1, int read_data_2, int immediate, char write_reg);

// global variables
// holds all of the instructions from the input file
char * i_mem; // the current fetch function should get the correct instruction based on the pc value.

//defined in the fetch instructions
int pc = 0;
int next_pc = pc + 4;

// from the deocode instructions
int registerfile[32];
int jump_target = 0;

// from the execute instructions
char alu_op = 0; // closest we can get to 4 bits
char alu_zero = 0; 
int branch_target = 0;

// mem
int d_mem[32]; // can't make a var with '-', so used '_' instead

// writeback
int total_clock_cycles = 0;

// controlunit
int RegWrit = 0;
int RegDst = 0;
int Branch = 0;
int ALUSrc = 0;
int InstType = 0;
int MemWrite = 0;
int MemtoReg = 0;
int MemRead = 0;
int Jump = 0;


// Fetch fucnction: reads an instruction from the input text file, converts to int and sends output to data
void Fetch() {
    unsigned int data = i_mem[pc * 8 + pc / 4] == '1'; // the output 32 bits that make up the current instruction
    for (int i = (pc * 8 + pc / 4) + 1; i < 32; i++) { // converts the 32 chars from the input file to a 32 bit unsigned integer to be passed to id
        data = data << 1;
        data += i_mem[i] == '1';
    }
    // printf("%.8x\n", data);
    
    // we need to do more here to actually 
    if (Jump) {
        pc = jump_target;
    } else if (Branch & alu_zero) {
        pc = branch_target;
    } else {
        pc = next_pc;
    }
    next_pc = pc + 4;

    Decode(data);
}

// Decode function
void Decode (unsigned int data) {
    // first section just gets the operators for each instruction
    unsigned int opcode = isolate_bits(data, 26, 6); // gets the opcode and moves it to the lower bits, allowing simple comparisons to numbers / hex
    // printf("%.8x\n", opcode); // used for debug to ensure that opcode was extracted correctly
    // call control unit with the opcode
    short immediate = isolate_bits(data, 0, 16);
    // printf("%x\n", sign_extend(immediate)); // test for sign extend
    char rs = isolate_bits(data, 21, 5);
    char rt = isolate_bits(data, 16, 5);
    char rd = isolate_bits(data, 11, 5);
    char shamt = isolate_bits(data, 6, 5);
    char func = isolate_bits(data, 0, 6);
    
    jump_target = isolate_bits(data, 0, 26);
    // shift left 2
    jump_target = jump_target << 2;

    int read_data_1 = registerfile[rs]; // as far as I can tell, read data 1 is always rs
    int read_data_2 = registerfile[rt]; // as far as I can tell, read data 2 is always rt, unless modified after by ALUSrc
    
    char write_reg = RegDst ? rd : rt; // if RegDst is 1, write_reg should be rd. if not, rt.
    // I think that write_reg needs to be passed as an operator to writeback or made into a global variable.
    
    Execute (read_data_1, read_data_2, immediate, write_reg);
}

// I am basing the inputs to Execute on what hits the ID/EX buffer on page 12 of lecture-4_processor-3, excluding pc + 4 (pc_next) as that is a global variable
// write_reg is also only relevant in the writeback section, so it should be passed as an argument to all funtions until then, unless you want to change that
void Execute (int read_data_1, int read_data_2, int immediate, char write_reg) {
    if (ALUSrc) {
        read_data_2 = immediate;
    }


}

int main() {
    string input_file;
    cout << "Enter the program file name to run:\n" << endl;
    cin >> input_file;
    //get the input file
    ifstream input(input_file);

    // gets the length of the input file
    input.seekg (0, input.end); // these three lines were taken from https://www.cplusplus.com/reference/istream/istream/read/
    int length = input.tellg(); // the first line sets the istream to the last bit, the second line gets the curremt character position,
    input.seekg (0, input.beg); // and the third line resets the stream to the first char.

    // reads the input file and writes it to i_mem
    //cout << length << endl;
    i_mem = new char[length];
    input.read(i_mem, length);
    input.close();

    // need to initialize the regester file before begining the program.
    for (int i = 0; i < 32; i++) {
        registerfile[i] = 0;
        d_mem[i] = 0;
    }
    // manually define register or d-mem preexisting values here

    Fetch();

    delete i_mem;
    return 0;
}

// isolates bits in input from start to start + len - 1. IE isolate_bits(1010 0101, 2, 4) should return 0000 1001
// if this does not work, this must be fixed
unsigned int isolate_bits(unsigned int input, char start, char len) {
    unsigned int temp = (1 << len) - 1;
    return (input & (temp << start)) >> start;
}

int sign_extend(short input) {
    if (input & (1 << 16)) { // if the sign bit is 1
        return input | (1 << 16 - 1) << 16;
    }
    return input;
}