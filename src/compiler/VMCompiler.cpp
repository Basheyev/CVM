/*============================================================================
*
*  Virtual Machine Compiler class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "compiler/VMCompiler.h"
#include <iostream>


// TODO переменные и их вывод в консоль

using namespace vm;


VMCompiler::VMCompiler() {
	destImage = NULL;
	lexer = new VMLexer();
	currentToken = 0;
}


VMCompiler::~VMCompiler() {
	delete lexer;
}


void VMCompiler::compile(const char* sourceCode, ExecutableImage* destImage) {
	lexer->parseToTokens(sourceCode);
	currentToken = 0;
	this->destImage = destImage;
	parseExpression();
	destImage->disassemble();
}


//------------------------------------------------------------------------------------------------
// Expression compiler
//------------------------------------------------------------------------------------------------
void VMCompiler::parseExpression() {
	Token tkn;
	parseTerm();
	tkn = lexer->getToken(currentToken);
	while (tkn.type==TokenType::PLUS || tkn.type==TokenType::MINUS) {
		currentToken++;
		parseTerm();
		if (tkn.type == TokenType::PLUS) {
			destImage->emit(OP_ADD);
		} else {
			destImage->emit(OP_SUB);
		}
		tkn = lexer->getToken(currentToken);
	}
}


void VMCompiler::parseTerm() {
	Token tkn;
	parseFactor();
	currentToken++;
	tkn = lexer->getToken(currentToken);
	while (tkn.type == TokenType::MULTIPLY || tkn.type == TokenType::DIVIDE) {
		currentToken++;
		parseFactor();
		if (tkn.type == TokenType::MULTIPLY) {
			destImage->emit(OP_MUL);
		} else {
			destImage->emit(OP_DIV);
		}
		currentToken++;
		tkn = lexer->getToken(currentToken);
	}

	
}

void VMCompiler::parseFactor() {
	Token tkn = lexer->getToken(currentToken);
	bool unaryMinus = false;
	if (tkn.type == TokenType::MINUS) {
		currentToken++;
		tkn = lexer->getToken(currentToken);
		unaryMinus = true;
	}

	if (unaryMinus) destImage->emit(OP_CONST, 0);

	if (tkn.type == TokenType::OP_PARENTHESES) {
		currentToken++;
		parseExpression();
	} else if (tkn.type == TokenType::CONST_INTEGER) {
		destImage->emit(OP_CONST, lexer->tokenToInt(tkn));
	}

	if (unaryMinus) destImage->emit(OP_SUB);
}


