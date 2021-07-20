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
#include "runtime/ExecutableImage.h"

using namespace std;
using namespace vm;


//-----------------------------------------------------------------------------
// Clears executable image
//-----------------------------------------------------------------------------
void ExecutableImage::clear() {
	image.clear();
	emitAddress = 0;
}

//-----------------------------------------------------------------------------
// Checks available space and resize image if required
//-----------------------------------------------------------------------------
void ExecutableImage::prepareSpace(WORD wordsCount) {
	WORD required = emitAddress + wordsCount;
	if (image.size() < required) image.resize(required);
}

void ExecutableImage::prepareSpace(WORD address, WORD wordsCount) {
	WORD required = address + wordsCount;
	if (image.size() < required) image.resize(required);
}


//-----------------------------------------------------------------------------
// Sets EmitAddress value to specified address
//-----------------------------------------------------------------------------
WORD ExecutableImage::setEmitAddress(WORD address) {
	return emitAddress = address;
}


//-----------------------------------------------------------------------------
// Returns EmitAddress value
//-----------------------------------------------------------------------------
WORD ExecutableImage::getEmitAddress() {
	return emitAddress;
}


//-----------------------------------------------------------------------------
// Writes opcode to executable image at current EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::emit(WORD opcode) {
	prepareSpace(1);
	WORD startAddress = emitAddress;
	image[emitAddress++] = opcode;
	return startAddress;
}

//-----------------------------------------------------------------------------
// Writes opcode and its operand to executable image at current EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::emit(WORD opcode, WORD operand) {
	prepareSpace(2);
	WORD startAddress = emitAddress;
	image[emitAddress++] = opcode;
	image[emitAddress++] = operand;
	return startAddress;
}

//-----------------------------------------------------------------------------
// Writes opcode and its operands to executable image at current EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::emit(WORD opcode, WORD operand1, WORD operand2) {
	prepareSpace(3);
	WORD startAddress = emitAddress;
	image[emitAddress++] = opcode;
	image[emitAddress++] = operand1;
	image[emitAddress++] = operand2;
	return startAddress;
}


//-----------------------------------------------------------------------------
// Writes specified image to this executable image at current EmitAddress
//-----------------------------------------------------------------------------
WORD ExecutableImage::emit(ExecutableImage& img) {
	WORD startAddress = emitAddress;
	WORD bytesCount = img.getImageSize();
	WORD wordsCount = bytesCount / sizeof(WORD);
	prepareSpace(wordsCount);
	memcpy(image.data() + emitAddress, img.getImage(), bytesCount);
	emitAddress += wordsCount;
	return startAddress;
}


//-----------------------------------------------------------------------------
// Write WORD to specified memory address
//-----------------------------------------------------------------------------
void ExecutableImage::writeWord(WORD address, WORD value) {
	WORD temp = emitAddress;
	prepareSpace(address, 1);
	image[address] = value;
	emitAddress = temp;
}


//-----------------------------------------------------------------------------
// Writes data to executable image at current EmitAddress
//-----------------------------------------------------------------------------
void ExecutableImage::writeData(WORD address, void* data, WORD bytesCount) {
	WORD reminder = bytesCount % sizeof(WORD);
	WORD wordsCount = bytesCount / sizeof(WORD);
	if (reminder != 0) wordsCount++;
	prepareSpace(address, wordsCount);
	memcpy(image.data() + address, data, bytesCount);
}


//-----------------------------------------------------------------------------
// Returns pointer to executable image
//-----------------------------------------------------------------------------
WORD* ExecutableImage::getImage() {
	return image.data();
}


//-----------------------------------------------------------------------------
// Returns memory size in bytes
//-----------------------------------------------------------------------------
WORD ExecutableImage::getImageSize() {
	return image.size() * sizeof(WORD);
}


//-----------------------------------------------------------------------------
// Disassembles executable image to console
//-----------------------------------------------------------------------------
void ExecutableImage::disassemble() {
	cout << "-----------------------------------------------------" << endl;
	cout << "Virtual machine executable image disassembly" << endl;
	cout << "-----------------------------------------------------" << endl;
	WORD opcode;
	WORD previousOp = -1;
	WORD ip = 0;
	do {
		opcode = image[ip];
		if (opcode != OP_HALT) ip += printMnemomic(ip);
		else {
			if (previousOp != OP_HALT) printMnemomic(ip);
			ip++;
		}
		previousOp = opcode;
	} while (ip < image.size());
}


//-----------------------------------------------------------------------------
// Prints instruction mnemonic
//-----------------------------------------------------------------------------
WORD ExecutableImage::printMnemomic(WORD address) {
	WORD ip = address;
	WORD opcode = image[ip++];
	cout << "[" << setw(6) << address << "]    ";
	switch (opcode) {
		//------------------------------------------------------------------------
		// STACK OPERATIONS
		//------------------------------------------------------------------------
		case OP_CONST:	cout << "iconst  " << image[ip++]; break;
		case OP_PUSH:   cout << "ipush   [" << image[ip++] << "]"; break;
		case OP_POP:    cout << "ipop    [" << image[ip++] << "]"; break;
		case OP_DUP:    cout << "idup    " ; break;
		//------------------------------------------------------------------------
		// ARITHMETIC OPERATIONS
		//------------------------------------------------------------------------
		case OP_INC:    cout << "iinc    "; break;
		case OP_DEC:    cout << "idec    "; break;
		case OP_ADD:    cout << "iadd    "; break;
		case OP_SUB:    cout << "isub    "; break;
		case OP_MUL:    cout << "imul    "; break;
		case OP_DIV:    cout << "idiv    "; break;
		//------------------------------------------------------------------------
		// BITWISE OPERATIONS
		//------------------------------------------------------------------------
		case OP_AND:    cout << "iand    "; break;
		case OP_OR:     cout << "ior     "; break;
		case OP_XOR:    cout << "ixor    "; break;
		case OP_NOT:    cout << "inot    "; break;
		case OP_SHL:    cout << "ishl    "; break;
		case OP_SHR:    cout << "ishr    "; break;
		//------------------------------------------------------------------------
		// FLOW CONTROL OPERATIONS
		//------------------------------------------------------------------------
		case OP_JMP:    cout << "jmp     [" << image[ip++] << "]"; break;
		case OP_JE:     cout << "je      [" << image[ip++] << "]"; break;
		case OP_JNE:    cout << "jne     [" << image[ip++] << "]"; break;
		case OP_JG:     cout << "jg      [" << image[ip++] << "]"; break;
		case OP_JGE:    cout << "jge     [" << image[ip++] << "]"; break;
		case OP_JL:     cout << "jl      [" << image[ip++] << "]"; break;
		case OP_JLE:    cout << "jle     [" << image[ip++] << "]"; break;
		//------------------------------------------------------------------------
		// PROCEDURE CALL OPERATIONS
		//------------------------------------------------------------------------
		case OP_CALL:   cout << "call    [" << image[ip++] << "], " << image[ip++]; break;
		case OP_RET:    cout << "ret     "; break;
		case OP_SYSCALL:cout << "syscall 0x" << setbase(16) << image[ip++] << setbase(10); break;
		case OP_HALT: 	cout << "---- halt ----"; break;
		//------------------------------------------------------------------------
		// LOCAL VARIABLES AND ARGUMENTS ACCESS OPERATIONS
		//------------------------------------------------------------------------
		case OP_LOAD:	cout << "iload   #" << image[ip++]; break;
		case OP_STORE:	cout << "istore  #" << image[ip++]; break;
		case OP_ARG:	cout << "iarg    #" << image[ip++]; break;
		case OP_DROP:	cout << "idrop   "; break;
	default:
		cout << "0x" << setbase(16) << opcode << setbase(10);
	}
	cout << endl;
	return ip - address;
}
