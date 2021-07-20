/*============================================================================
*
*  Virtual Machine Executable Image class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include <vector>
#include "VirtualMachine.h"

using namespace std;

namespace vm {

	class ExecutableImage {
	public:
		void clear();
		WORD setEmitAddress(WORD address);
		WORD getEmitAddress();
		WORD emit(WORD opcode);
		WORD emit(WORD opcode, WORD operand);
		WORD emit(WORD opcode, WORD operand1, WORD operand2);
		WORD emit(ExecutableImage& img);
		void writeWord(WORD address, WORD value);
		void writeData(WORD address, void* data, WORD bytesCount);
		WORD* getImage();
		WORD getImageSize();
		void disassemble();

	private:
		vector<WORD> image;
		WORD emitAddress;
		void prepareSpace(WORD wordsCount);
		void prepareSpace(WORD address, WORD wordsCount);
		WORD printMnemomic(WORD address);
	};


};