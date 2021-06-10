/*============================================================================
*
*  Virtual Machine Compiler class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "runtime/VMRuntime.h"
#include "image/VMImage.h"
#include "compiler/VMLexer.h"
#include <vector>

using namespace std;

namespace vm {

	class VMCompiler {
	public:
		VMCompiler();
		~VMCompiler();
		void compile(const char* sourceCode, VMImage* destImage);

	private:
		VMImage* destImage;
		VMLexer* lexer;
		size_t currentToken;

		void parseExpression();
		void parseTerm();
		void parseFactor();
	};
};