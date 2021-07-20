/*============================================================================
*
*  Virtual Machine Compiler class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "runtime/VirtualMachine.h"
#include "runtime/ExecutableImage.h"
#include "compiler/VMLexer.h"
#include <vector>

using namespace std;

namespace vm {

	class VMCompiler {
	public:
		VMCompiler();
		~VMCompiler();
		void compile(const char* sourceCode, ExecutableImage* destImage);

	private:
		ExecutableImage* destImage;
		VMLexer* lexer;
		size_t currentToken;

		void parseExpression();
		void parseTerm();
		void parseFactor();
	};
};