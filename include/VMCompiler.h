/*============================================================================
*
*  Virtual Machine Compiler class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "VMRuntime.h"
#include "VMImage.h"
#include "VMLexer.h"
#include <vector>

using namespace std;

namespace vm {

	class VMCompiler {
	public:
		VMCompiler();
		~VMCompiler();
		void compile(const char* sourceCode, VMImage* destImage);

	private:
		VMLexer* parser;
		size_t currentToken;

		void parseExpression(size_t startIndex, VMImage* destImage);
		void parseTerm(VMImage* destImage);
		void parseFactor(VMImage* destImage);
	};
};