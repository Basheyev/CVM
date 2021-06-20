/*============================================================================
*
*  Virtual Machine Executable Image class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "VMRuntime.h"

#pragma once

namespace vm {


	class VMImage {
	public:

		VMImage();
		~VMImage();

		void clear();

		WORD setEmitPointer(WORD address);
		WORD getEmitPointer();
		WORD emit(WORD opcode);
		WORD emit(WORD opcode, WORD operand);
		WORD emit(WORD opcode, WORD operand1, WORD operand2);
		
		WORD readWord(WORD address);
		void writeWord(WORD address, WORD value);
		WORD writeData(WORD address, void* data, WORD bytesCount);
				
		void* getImage();
		size_t getImageSize();

		void disassemble();
		static WORD printMnemomic(WORD* memory, WORD virtualAddress);

	private:

		WORD memory[MAX_MEMORY];
		WORD imageSize;
		WORD ep;

	};


};