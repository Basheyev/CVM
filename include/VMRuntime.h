/*============================================================================
* 
*  Virtual Machine class header
* 
*  Lightweight embeddable 32-bit stack virtual machine
*  
* 
*  (C) Bolat Basheyev 2021
* 
============================================================================*/

#pragma once

namespace vm {

	typedef __int32 WORD;

	constexpr WORD OP_CODE_MASK = 0b00000000000000000000000011111111;
	constexpr WORD OP_TYPE_MASK = 0b00000000000000000000111000000000;

	constexpr WORD OP_HALT      = 0b00000000000000000000000000000000;
	constexpr WORD OP_CONST     = 0b00000000000000000000000000000001;
	constexpr WORD OP_PUSH      = 0b00000000000000000000000000000010;
	constexpr WORD OP_POP       = 0b00000000000000000000000000000011;

	constexpr WORD OP_INC       = 0b00000000000000000000000000000100;
	constexpr WORD OP_DEC       = 0b00000000000000000000000000000101;
	constexpr WORD OP_ADD       = 0b00000000000000000000000000000110;
	constexpr WORD OP_SUB       = 0b00000000000000000000000000000111;
	constexpr WORD OP_MUL       = 0b00000000000000000000000000001000;
	constexpr WORD OP_DIV       = 0b00000000000000000000000000001001;

	constexpr WORD OP_AND       = 0b00000000000000000000000000001010;
	constexpr WORD OP_OR        = 0b00000000000000000000000000001011;
	constexpr WORD OP_XOR       = 0b00000000000000000000000000001100;
	constexpr WORD OP_NOT       = 0b00000000000000000000000000001101;
	constexpr WORD OP_SHL       = 0b00000000000000000000000000001110;
	constexpr WORD OP_SHR       = 0b00000000000000000000000000001111;

	constexpr WORD OP_JMP       = 0b00000000000000000000000000010001;
	constexpr WORD OP_CMPJE     = 0b00000000000000000000000000010010;
	constexpr WORD OP_CMPJNE    = 0b00000000000000000000000000010011;
	constexpr WORD OP_CMPJG     = 0b00000000000000000000000000010100;
	constexpr WORD OP_CMPJGE    = 0b00000000000000000000000000010101;
	constexpr WORD OP_CMPJL     = 0b00000000000000000000000000010110;
	constexpr WORD OP_CMPJLE    = 0b00000000000000000000000000010111;

	constexpr WORD OP_DUP       = 0b00000000000000000000000000011000;
	constexpr WORD OP_CALL      = 0b00000000000000000000000000011001;
	constexpr WORD OP_RET       = 0b00000000000000000000000000011010;
	constexpr WORD OP_SYSCALL   = 0b00000000000000000000000000011011;

	constexpr WORD OP_RESERVED1 = 0b00000000000000000000000000011100;
	constexpr WORD OP_RESERVED2 = 0b00000000000000000000000000011101;
	constexpr WORD OP_RESERVED3 = 0b00000000000000000000000000011110;
	constexpr WORD OP_RESERVED4 = 0b00000000000000000000000000011111;

	constexpr WORD MAX_MEMORY   = 65536;



	class VMRuntime {
	public:

		VMRuntime();                                // Constructor
		~VMRuntime();                               // Desctructor

		bool loadImage(void* image, size_t size);   // Load executable image

		void run();                                 // Runs image from address 0

		WORD readWord(WORD address);                // Read WORD from memory
		void writeWord(WORD address, WORD value);   // Write WORD to memory 

		WORD getMaxAddress();                       // Get max address in 32-bit words
		WORD getIP();                               // Get Instruction Pointer address
		WORD getSP();                               // Get Stack Pointer address

	private:

		WORD  memory[MAX_MEMORY];                   // Random access memory array
		WORD  ip;                                   // Instruction pointer
		WORD  sp;                                   // Stack pointer
		WORD  fp;                                   // Frame pointer
	
		void systemCall(WORD n);                    // System call (interruption)
		void printState();                          // Print current VM state
	};

};