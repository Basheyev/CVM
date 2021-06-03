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
#include "VMRuntime.h"
#include "VMImage.h"


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
			memory[--sp] = memory[memory[ip++]];
			break;
		case OP_POP:  
		    memory[memory[ip++]] = memory[sp++]; 
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
			a = memory[ip++];
			memory[--sp] = ip;       
			ip = a;                  
			break;
		case OP_RET:
			ip = memory[sp++];       
			break;
		case OP_SYSCALL:
			a = memory[ip++];
			systemCall(a);
			break;
		case OP_HALT: 
			printState();
		    return;
		default:
			cout << "Runtime error - unknown opcode=" << opcode << endl;
			printState();
			return;
		}

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
	cout << "IP=" << ip << " SP=" << sp << " STACK=[";
	for (WORD i = MAX_MEMORY - 2; i >= sp; i--) {
		cout << memory[i];
		if (i > sp) cout << ",";
	}
	cout << "] -> TOP" << endl;
}
