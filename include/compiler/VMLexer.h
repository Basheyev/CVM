/*============================================================================
*
*  Virtual Machine Compiler lexer class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/

#pragma once

#include "runtime/VMRuntime.h"
#include <vector>

using namespace std;

namespace vm {

	constexpr char* BLANKS = "\x20\n\r\t";
	constexpr char* DELIMETERS = ",;{}[]()=><+-*/&|~^!";

	enum class TokenType {
		NONE = 0, UNKNOWN, IDENTIFIER,
		CONST_CHAR, CONST_INTEGER, CONST_REAL, CONST_STRING,
		COMMA, MEMBER_ACCESS, EOS, 
		OP_BRACES, CL_BRACES, OP_BRACKETS, CL_BRACKETS, OP_PARENTHESES, CL_PARENTHESES,
		MODULE, BYTE, SHORT, INT, LONG, CHAR, FLOAT, DOUBLE, POINTER, 
		IF, ELSE, WHILE, RETURN,
		ASSIGN, PLUS, MINUS, MULTIPLY, DIVIDE, AND, OR, XOR, NOT, SHL, SHR,
		EQUAL, NOT_EQUAL, GREATER, GR_EQUAL, LESS, LS_EQUAL, LOGIC_AND, LOGIC_OR, LOGIC_NOT
	};

	constexpr char* const TOKEN_TYPE_MNEMONIC[] = {
		"NONE", "UNKNOWN", "IDENTIFIER",
		"CONST_CHAR", "CONST_INT", "CONST_FLOAT", "CONST_STRING",
		"COMMA", "MEMBER_ACCESS", "EOS",
		"OP_BRACES", "CL_BRACES", "OP_BRACKETS", "CL_BRACKETS", "OP_PARENTHESES", "CL_PARENTHESES",
		"MODULE", "BYTE", "SHORT", "INT", "LONG", "CHAR", "FLOAT", "DOUBLE", "POINTER", "IF", "ELSE", "WHILE", "RETURN",
		"ASSIGN", "EQUAL", "NOT_EQUAL", "GREATER", "GR_EQUAL", "LESS", "LS_EQUAL",
		"PLUS", "MINUS", "MUL", "DIV", "AND", "OR", "XOR", "NOT", "SHL", "SHR",
		"L_AND", "L_OR", "L_NOT"
	};

	typedef struct {
		TokenType type;          
		char* text;              
		WORD length;             
		WORD row;                
		WORD col;                
	} Token;

	constexpr Token EMPTY_TOKEN = { TokenType::NONE, "", 0, 0, 0 };

	class VMLexer {
	public:

		VMLexer();
		~VMLexer();

		void parseToTokens(const char* sourceCode);

		Token getToken(size_t index);
		WORD tokenToInt(Token& tkn);
		void printToken(Token& tkn);
		void printAllTokens();
		size_t getTokenCount();


	private:

		vector<Token>* tokens;
		WORD rowCounter;
		char* rowPointer;

		bool isBlank(char value);
		bool isDelimeter(char value);
		TokenType pushToken(char* text, size_t length);
		TokenType getTokenType(char* text, size_t length);
		TokenType identifyNumber(char* text, size_t length);
		TokenType identifyKeyword(char* text, size_t length);

	};
};