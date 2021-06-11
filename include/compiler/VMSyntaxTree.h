/*============================================================================
*
*  Abstract Syntax Tree class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include "compiler/VMLexer.h"
#include "compiler/VMNode.h"

namespace vm {

	class VMSyntaxTree {
	public:

		VMSyntaxTree();
		~VMSyntaxTree();

		VMNode* parse(const char* source);

	private:

		VMLexer* lexer;
		VMNode* root;
		size_t currentToken;

		VMNode* parseFunction();
		VMNode* parseDeclaration();
		VMNode* parseStatement();
		VMNode* parseAssignment();
		VMNode* parseExpression();
		VMNode* parseTerm();
		VMNode* parseFactor();

	};


}