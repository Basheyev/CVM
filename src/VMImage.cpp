/*============================================================================
*
*  Virtual Machine Executable Image class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/

#include <cstring>
#include <iostream>
#include <iomanip>
#include "VMImage.h"

using namespace std;
using namespace vm;


VMImage::VMImage() {
	clear();
}


VMImage::~VMImage() {

}


void VMImage::clear() {
	memset(memory, 0, MAX_MEMORY);
	imageSize = 0;
	ep = 0;
}


/*
 Sets Emit Pointer (EP) value to specified address
*/
WORD VMImage::setEmitPointer(WORD address) {
	if (address >= MAX_MEMORY - 1) return ep;
	return ep = address;
}



/*
 Return Emit Pointer (EP) value
*/
WORD VMImage::getEmitPointer() {
	return ep;
}



/*
 Writes opcode to the current EP address and increments EP
*/
WORD VMImage::emit(WORD opcode) {
	WORD opAddr = ep;
	memory[ep++] = opcode;
	if (ep > imageSize) imageSize = ep;
	return opAddr;
}


/*
 Sequentially writes opcode and operand starting from the current EP address and increments EP by 2
*/
WORD VMImage::emit(WORD opcode, WORD operand) {
	WORD opAddr = ep;
	memory[ep++] = opcode;
	memory[ep++] = operand;
	if (ep > imageSize) imageSize = ep;
	return opAddr;
}

/*
 Writes data to specified address
*/
WORD VMImage::writeData(WORD address, void* data, size_t length) {
	memcpy(&memory[address], data, length);
	if (address + length > imageSize) imageSize = address + length;
	return address;
}


/*
 Read memory (WORD)
*/
WORD VMImage::readWord(WORD address) {
	return memory[address];
}


/*
 Write memory (WORD)
*/
void VMImage::writeWord(WORD address, WORD value) {
	memory[address] = value;
	if (address > imageSize) imageSize = address;
}


/*
 Returns memory size in 32-bit DWORDs
*/
size_t VMImage::getImageSize() {
	return ((size_t)imageSize + 1) * sizeof(WORD);
}


/*
 Returns pointer to executable image
*/
void* VMImage::getImage() {
	return (void*)memory;
}



void VMImage::dissasemble() {
	WORD opcode;
	WORD previousOp = 0xFFFFFFFF;
	WORD ip = 0;

	cout << "-----------------------------------------------------" << endl;
	cout << "Virtual machine image dissasembly" << endl;
	cout << "-----------------------------------------------------" << endl;
	while (ip < imageSize) {

		opcode = memory[ip];
		if (opcode != OP_HALT || previousOp != opcode) cout << "[" << setw(4) << ip << "]    ";
		ip++;

		switch (opcode) {
		//------------------------------------------------------------------------
		// STACK OPERATIONS
		//------------------------------------------------------------------------
		case OP_CONST:	cout << "iconst  " << memory[ip++] << endl; break;
		case OP_PUSH:   cout << "ipush   [" << memory[ip++] << "]" << endl; break;
		case OP_POP:    cout << "ipop    [" << memory[ip++] << "]" << endl; break;
		case OP_DUP:    cout << "idup    " << endl; break;
		//------------------------------------------------------------------------
		// ARITHMETIC OPERATIONS
		//------------------------------------------------------------------------
		case OP_INC: cout << "iinc    " << endl; break;
		case OP_DEC: cout << "idec    " << endl; break;
		case OP_ADD: cout << "iadd    " << endl; break;
		case OP_SUB: cout << "isub    " << endl; break;
		case OP_MUL: cout << "imul    " << endl; break;
		case OP_DIV: cout << "idiv    " << endl; break;
		//------------------------------------------------------------------------
		// BITWISE OPERATIONS
		//------------------------------------------------------------------------
		case OP_AND: cout << "iand    " << endl; break;
		case OP_OR:  cout << "ior     " << endl; break;
		case OP_XOR: cout << "ixor    " << endl; break;
		case OP_NOT: cout << "inot    " << endl; break;
		case OP_SHL: cout << "ishl    " << endl; break;
		case OP_SHR: cout << "ishr    " << endl; break;
		//------------------------------------------------------------------------
		// FLOW CONTROL OPERATIONS
		//------------------------------------------------------------------------
		case OP_JMP:   cout << "jmp     " << memory[ip++] << endl; break;
		case OP_CMPJE: cout << "icmpje  " << memory[ip++] << endl; break;
		case OP_CMPJNE:cout << "icmpjne " << memory[ip++] << endl; break;
		case OP_CMPJG: cout << "icmpjg  " << memory[ip++] << endl; break;
		case OP_CMPJGE:cout << "icmpjge " << memory[ip++] << endl; break;
		case OP_CMPJL: cout << "icmpjl  " << memory[ip++] << endl; break;
		case OP_CMPJLE:cout << "icmpjle " << memory[ip++] << endl; break;
		//------------------------------------------------------------------------
		// PROCEDURE CALL OPERATIONS
		//------------------------------------------------------------------------
		case OP_CALL:    cout << "call    [" << memory[ip++] << "]" << endl; break;
		case OP_RET:     cout << "ret     " << endl; break;
		case OP_SYSCALL: cout << "syscall 0x" << setbase(16) << memory[ip++] << setbase(10) << endl; break;
		case OP_HALT: 	 if (previousOp != opcode) cout << "---- halt ----" << endl; break;
		default:
			cout << "0x" << setbase(16) << opcode << setbase(10) << endl;
		}

		previousOp = opcode;
	}
}
