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
#include "image/VMImage.h"

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
 Sequentially writes opcode and operand starting from the current EP address and increments EP by 2
*/
WORD VMImage::emit(WORD opcode, WORD operand1, WORD operand2) {
	WORD opAddr = ep;
	memory[ep++] = opcode;
	memory[ep++] = operand1;
	memory[ep++] = operand2;
	if (ep > imageSize) imageSize = ep;
	return opAddr;
}


/*
 Writes data to specified address
*/
WORD VMImage::writeData(WORD address, void* data, WORD bytesCount) {
	WORD wordsCount = bytesCount / sizeof(WORD);
	memcpy(&memory[address], data, bytesCount);
	if ((address + wordsCount) > imageSize) imageSize = address + wordsCount;
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



void VMImage::disassemble() {
	WORD opcode;
	WORD previousOp = 0xFFFFFFFF;
	WORD ip = 0;
	bool haltFlag = false;

	cout << "-----------------------------------------------------" << endl;
	cout << "Virtual machine image dissasembly" << endl;
	cout << "-----------------------------------------------------" << endl;
	WORD offset;
	while (ip < imageSize) {
		opcode = memory[ip];
		if (opcode != 0) {
			offset = printMnemomic(&memory[ip], ip);
			cout << endl;
			ip += offset;
			haltFlag = false;
		}
		else {
			if (haltFlag == false) printMnemomic(&memory[ip], ip), cout << endl;
			ip++;
			haltFlag = true;
		}
		previousOp = opcode;
	}
}



WORD VMImage::printMnemomic(WORD* memory, WORD virtualAddress) {
	
	static WORD previousOp = 0xFFFFFFFF;

	WORD opcode;
	WORD ip = 0;

	opcode = memory[ip];
	if (opcode != OP_HALT || previousOp != opcode) cout << "[" << setw(4) << virtualAddress << "]    ";
	ip++;

	switch (opcode) {
		//------------------------------------------------------------------------
		// STACK OPERATIONS
		//------------------------------------------------------------------------
	case OP_CONST:	cout << "iconst  " << memory[ip++]; break;
	case OP_PUSH:   cout << "ipush   [" << memory[ip++] << "]"; break;
	case OP_POP:    cout << "ipop    [" << memory[ip++] << "]"; break;
	case OP_DUP:    cout << "idup    " ; break;
		//------------------------------------------------------------------------
		// ARITHMETIC OPERATIONS
		//------------------------------------------------------------------------
	case OP_INC: cout << "iinc    "; break;
	case OP_DEC: cout << "idec    "; break;
	case OP_ADD: cout << "iadd    "; break;
	case OP_SUB: cout << "isub    "; break;
	case OP_MUL: cout << "imul    "; break;
	case OP_DIV: cout << "idiv    "; break;
		//------------------------------------------------------------------------
		// BITWISE OPERATIONS
		//------------------------------------------------------------------------
	case OP_AND: cout << "iand    "; break;
	case OP_OR:  cout << "ior     "; break;
	case OP_XOR: cout << "ixor    "; break;
	case OP_NOT: cout << "inot    "; break;
	case OP_SHL: cout << "ishl    "; break;
	case OP_SHR: cout << "ishr    "; break;
		//------------------------------------------------------------------------
		// FLOW CONTROL OPERATIONS
		//------------------------------------------------------------------------
	case OP_JMP:   cout << "jmp     [" << memory[ip++] << "]"; break;
	case OP_CMPJE: cout << "icmpje  [" << memory[ip++] << "]"; break;
	case OP_CMPJNE:cout << "icmpjne [" << memory[ip++] << "]"; break;
	case OP_CMPJG: cout << "icmpjg  [" << memory[ip++] << "]"; break;
	case OP_CMPJGE:cout << "icmpjge [" << memory[ip++] << "]"; break;
	case OP_CMPJL: cout << "icmpjl  [" << memory[ip++] << "]"; break;
	case OP_CMPJLE:cout << "icmpjle [" << memory[ip++] << "]"; break;
		//------------------------------------------------------------------------
		// PROCEDURE CALL OPERATIONS
		//------------------------------------------------------------------------
	case OP_CALL:    cout << "call    [" << memory[ip++] << "], " << memory[ip++]; break;
	case OP_RET:     cout << "ret     "; break;
	case OP_SYSCALL: cout << "syscall 0x" << setbase(16) << memory[ip++] << setbase(10); break;
	case OP_HALT: 	 if (previousOp != opcode) cout << "---- halt ----"; break;
	case OP_LOAD:	cout << "iload   #" << memory[ip++]; break;
	case OP_STORE:	cout << "istore  #" << memory[ip++]; break;
	case OP_ARG:	cout << "iarg    #" << memory[ip++]; break;
	case OP_DROP:	cout << "idrop   "; break;
	default:
		cout << "0x" << setbase(16) << opcode << setbase(10);
	}

	previousOp = opcode;

	return ip;
}
