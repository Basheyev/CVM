/*============================================================================
*
*  Virtual Machine class implementation
*
*  Lightweight embeddable 32-bit stack virtual machine runtime.
*
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include <iostream>
#include <cstring>
#include "runtime/VMRuntime.h"
#include "image/VMImage.h"


using namespace std;
using namespace vm;


VMRuntime::VMRuntime() {
	memset(memory, 0, MAX_MEMORY);
	ip = 0;
	sp = MAX_MEMORY - 1;
}


VMRuntime::~VMRuntime() {

}


bool VMRuntime::loadImage(void* image, size_t size) {
	if (size > MAX_MEMORY * sizeof(WORD)) return false;
	memcpy(memory, image, size);
	ip = 0;
	sp = MAX_MEMORY - 1;
	fp = sp;
	return true;
}

//----------------------------------------------------------------------------
// Virtual Machine Runtime
//----------------------------------------------------------------------------
void VMRuntime::run() {
	WORD a, b;
	WORD opcode;

	ip = 0;
	sp = MAX_MEMORY - 1;
	fp = sp;

	while (1) {

		opcode = memory[ip++];
		
		switch (opcode) {
		//------------------------------------------------------------------------
		// STACK OPERATIONS
		//------------------------------------------------------------------------
		case OP_CONST: 
		    memory[--sp] = memory[ip++]; 
			break;
		case OP_PUSH:
			a = memory[ip++];
			memory[--sp] = memory[a];
			break;
		case OP_POP:  
			a = memory[ip++];
		    memory[a] = memory[sp++]; 
			break;
		case OP_DUP:
			a = memory[sp];
			memory[--sp] = a;
			break;
		//------------------------------------------------------------------------
		// ARITHMETIC OPERATIONS
		//------------------------------------------------------------------------
		case OP_INC:
			memory[sp]++;
			break;
		case OP_DEC:
			memory[sp]--;
			break;
		case OP_ADD:  
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a + b;
			break;
		case OP_SUB:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a - b;
			break;
		case OP_MUL:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a * b;
			break;
		case OP_DIV:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a / b;
			break;
		//------------------------------------------------------------------------
		// BITWISE OPERATIONS
		//------------------------------------------------------------------------
		case OP_AND:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a & b;
			break;
		case OP_OR:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a | b;
			break;
		case OP_XOR:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a ^ b;
			break;
		case OP_NOT:
			a = memory[sp++];
			memory[--sp] = ~a;
			break;
		case OP_SHL:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a << b;
			break;
		case OP_SHR:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a >> b;
			break;
		//------------------------------------------------------------------------
		// FLOW CONTROL OPERATIONS
		//------------------------------------------------------------------------
		case OP_JMP:
			ip = memory[ip];
			break;
		case OP_CMPJE:
			b = memory[sp++];
			a = memory[sp++];
			if (a == b) ip = memory[ip]; else ip++;
			break;
		case OP_CMPJNE:
			b = memory[sp++];
			a = memory[sp++];
			if (a != b) ip = memory[ip]; else ip++;
			break;
		case OP_CMPJG:
			b = memory[sp++];
			a = memory[sp++];
			if (a > b) ip = memory[ip]; else ip++;
			break;
		case OP_CMPJGE:
			a = memory[sp++];
			b = memory[sp++];
			if (a >= b) ip = memory[ip]; else ip++;
			break;
		case OP_CMPJL:
			b = memory[sp++];
			a = memory[sp++];
			if (a < b) ip = memory[ip]; else ip++;
			break;
		case OP_CMPJLE:
			b = memory[sp++];
			a = memory[sp++];
			if (a <= b) ip = memory[ip]; else ip++;
			break;
		//------------------------------------------------------------------------
		// PROCEDURE CALL OPERATIONS
		//------------------------------------------------------------------------
		case OP_CALL:
			a = memory[ip++];      // get call address and increment address
			memory[--sp] = ip;     // push return address to the stack
			memory[--sp] = fp;     // push old Frame pointer to stack
			fp = sp;               // save Stack pointer to new Frame pointer register
			ip = a;                // jump to call address
			break;
		case OP_RET:
			sp = fp;               // set stack pointer to Frame pointer
			fp = memory[sp++];     // pop previous Frame pointer
			ip = memory[sp++];     // jump to return address
			break;
		case OP_SYSCALL:
			a = memory[ip++];      // read system call index from top of the stack
			systemCall(a);         // make system call by index
			break;
		case OP_HALT: 
			printState();
		    return;
		//------------------------------------------------------------------------
		// LOCAL VARIABLES AND CALL ARGUMENTS OPERATIONS (x86 like convention)
		//------------------------------------------------------------------------
		case OP_LOAD:
			a = memory[ip++];         // read local variable index
			b = fp - a - 1;           // calculate local variable address
			memory[--sp] = memory[b]; // push local variable to stack
			break;
		case OP_STORE:
			a = memory[ip++];         // read local variable index
			b = fp - a - 1;           // calculate local variable address
			memory[b] = memory[sp++]; // pop top of stack to local variable
			break;
		case OP_ARG:
			a = memory[ip++];         // read parameter index
			b = fp + a + 2;           // calculate parameter address
			memory[--sp] = memory[b]; // push parameter to stack
			break;
		case OP_DROP:                 // pop and drop value from stack
			sp++;
			break;
		default:
			cout << "Runtime error - unknown opcode=" << opcode << endl;
			printState();
			return;
		}

		printState();
	}

}


void VMRuntime::systemCall(WORD n) {
	WORD ptr;
	switch (n) {
	case 0x20:  // print C style string
		ptr = memory[sp++];
		cout << ((char*)&memory[ptr]);
		break;
	}
}


/*
 Reads WORD from specified address in VM memory
*/
WORD VMRuntime::readWord(WORD address) {
	return memory[address];
}


/*
 Writes WORD to specified address in VM memory
*/
void VMRuntime::writeWord(WORD address, WORD value) {
	memory[address] = value;
}



/*
 Returns maximum RAM address (32-bit WORDs)
*/
WORD VMRuntime::getMaxAddress() {
	return MAX_MEMORY - 1;
}


/*
 Returns current Instruction Pointer (IP) value
*/
WORD VMRuntime::getIP() {
	return ip;
}


/*
 Returns  current Stack Pointer (IP) value
*/
WORD VMRuntime::getSP() {
	return sp;
}




/*
 Prints IP, SP and STACK value to standard out
*/
void VMRuntime::printState() {
	cout << "IP=" << ip << " FP=" << fp << " SP=" << sp << " STACK=[";
	for (WORD i = MAX_MEMORY - 2; i >= sp; i--) {
		cout << memory[i];
		if (i > sp) cout << ",";
	}
	cout << "] -> TOP" << endl;
}
