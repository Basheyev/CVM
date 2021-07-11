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

	typedef struct {
		Token& token;
		const char* msg;
	} ParserException;


	class VMParser {
	public:

		VMParser();
		~VMParser();

		VMNode* parse(const char* source);

	private:

		VMLexer* lexer;
		VMNode* root;
		size_t currentToken;

		VMNode* parseModule();
		VMNode* parseDeclaration();
		VMNode* parseFunction();
		VMNode* parseArguments();
		VMNode* parseCall();
		VMNode* parseBlock();
		VMNode* parseStatement();
		VMNode* parseIf();
		VMNode* parseWhile();

		VMNode* parseAssignment();
		VMNode* parseCondition();
		VMNode* parseExpression();
		VMNode* parseTerm();
		VMNode* parseFactor();

		inline bool next() { currentToken++; return currentToken < lexer->getTokenCount(); }
		inline Token getToken() { return lexer->getToken(currentToken); }
		inline Token getNextToken() { return lexer->getToken(currentToken + 1); }
		inline bool isTokenType(TokenType type) { return getToken().type == type; }
		inline bool isLogicOp(TokenType type) {	return type >= TokenType::EQUAL && type <= TokenType::LOGIC_NOT; }
		inline bool isDataType(TokenType type) { return type >= TokenType::BYTE && type <= TokenType::POINTER; }
		inline void raiseError(const char* msg) { throw ParserException{ getToken(), msg }; }
		inline void checkToken(TokenType type, const char* msg) { if (!isTokenType(type)) raiseError(msg); }

	};


}