/*============================================================================
*
*  Virtual Machine Executable Image class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/

#include <cstring>
#include "VMImage.h"


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
	return (imageSize + 1) * sizeof(WORD);
}


/*
 Returns pointer to executable image
*/
void* VMImage::getImage() {
	return (void*)memory;
}



void VMImage::dissasemble() {

}
