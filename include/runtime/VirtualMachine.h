/*============================================================================
* 
*  Virtual Machine class header
* 
*  Lightweight 32-bit stack virtual machine
*  
* 
*  (C) Bolat Basheyev 2021
* 
============================================================================*/

#pragma once

#include <vector>

using namespace std;

namespace vm {

	typedef __int32 WORD;

	constexpr WORD OP_CODE_MASK = 0b00000000000000000000000011111111;
	constexpr WORD OP_TYPE_MASK = 0b00000000000000000000111000000000;

	constexpr WORD OP_HALT      = 0b00000000000000000000000000000000;
	constexpr WORD OP_CONST     = 0b00000000000000000000000000000001;
	constexpr WORD OP_PUSH      = 0b00000000000000000000000000000010;
	constexpr WORD OP_POP       = 0b00000000000000000000000000000011;

	constexpr WORD OP_ADD       = 0b00000000000000000000000000000100;
	constexpr WORD OP_SUB       = 0b00000000000000000000000000000101;
	constexpr WORD OP_MUL       = 0b00000000000000000000000000000110;
	constexpr WORD OP_DIV       = 0b00000000000000000000000000000111;

	constexpr WORD OP_AND       = 0b00000000000000000000000000001000;
	constexpr WORD OP_OR        = 0b00000000000000000000000000001001;
	constexpr WORD OP_XOR       = 0b00000000000000000000000000001010;
	constexpr WORD OP_NOT       = 0b00000000000000000000000000001011;
	constexpr WORD OP_SHL       = 0b00000000000000000000000000001100;
	constexpr WORD OP_SHR       = 0b00000000000000000000000000001101;

	constexpr WORD OP_JMP       = 0b00000000000000000000000000001110;
	constexpr WORD OP_IFZERO    = 0b00000000000000000000000000001111;

	constexpr WORD OP_EQ        = 0b00000000000000000000000000010000;
	constexpr WORD OP_NE        = 0b00000000000000000000000000010001;
	constexpr WORD OP_GR        = 0b00000000000000000000000000010010;
	constexpr WORD OP_GE        = 0b00000000000000000000000000010011;
	constexpr WORD OP_LS        = 0b00000000000000000000000000010100;
	constexpr WORD OP_LE        = 0b00000000000000000000000000010101;
	constexpr WORD OP_LAND      = 0b00000000000000000000000000010110;
	constexpr WORD OP_LOR       = 0b00000000000000000000000000010111;
	constexpr WORD OP_LNOT      = 0b00000000000000000000000000011000;

	constexpr WORD OP_CALL      = 0b00000000000000000000000000011001;
	constexpr WORD OP_RET       = 0b00000000000000000000000000011010;
	constexpr WORD OP_SYSCALL   = 0b00000000000000000000000000011011;
	constexpr WORD OP_RESERVED  = 0b00000000000000000000000000011100;

	constexpr WORD OP_LOAD      = 0b00000000000000000000000000011101;
	constexpr WORD OP_STORE     = 0b00000000000000000000000000011110;
	constexpr WORD OP_ARG       = 0b00000000000000000000000000011111;


	class ExecutableImage {
	public:
		ExecutableImage();
		~ExecutableImage();
		void clear();
		WORD setEmitAddress(WORD address);
		WORD getEmitAddress();
		WORD emit(WORD opcode);
		WORD emit(WORD opcode, WORD operand);
		WORD emit(WORD opcode, WORD operand1, WORD operand2);
		WORD emit(ExecutableImage& img);
		void writeWord(WORD address, WORD value);
		void writeData(WORD address, void* data, WORD bytesCount);
		WORD readWord(WORD address);
		WORD* getImage();
		WORD getSize();
		void disassemble();

	private:
		vector<WORD> image;
		WORD emitAddress = 0;
		void prepareSpace(WORD wordsCount);
		void prepareSpace(WORD address, WORD wordsCount);
		WORD printMnemomic(WORD address);
	};


	class VirtualMachine {
	public:
		VirtualMachine(WORD memorySize = 0xFFFF);             // Allocates VM memory in bytes
		~VirtualMachine();                                    // Desctructor
		bool loadImage(ExecutableImage& image);               // Load executable image
		void execute();                                       // Runs image from address 0
		void printState();                                    // Print current VM state
		inline WORD getMaxAddress() { return maxAddress; };   // Get max address in WORDS
		inline WORD* getMemory() { return memory; };          // Returns pointer to VM RAM
		inline WORD getIP() { return ip; };                   // Get Instruction Pointer address
		inline WORD getSP() { return sp; };                   // Get Stack Pointer address
		inline WORD getFP() { return fp; };                   // Get Frame Pointer address
		inline WORD getLP() { return lp; };                   // Get Locals Pointer address
	private:
		WORD* memory;                                         // Random access memory array
		WORD  ip;                                             // Instruction pointer
		WORD  sp;                                             // Stack pointer
		WORD  fp;                                             // Frame pointer
		WORD  lp;                                             // Local variables pointer
		WORD  maxAddress;                                     // Highest address in words
		void sysCall(WORD n);                                 // System call
	};





};