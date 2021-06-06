/*============================================================================
*
*  Virtual Machine Compiler class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "VMCompiler.h"
#include <iostream>

using namespace vm;


VMCompiler::VMCompiler() {
	parser = new VMLexer();
	currentToken = 0;
}


VMCompiler::~VMCompiler() {
	delete parser;
}


void VMCompiler::compile(const char* sourceCode, VMImage* destImage) {
	parser->parseToTokens(sourceCode);
	currentToken = 0;
	this->destImage = destImage;
	parseExpression();
	destImage->dissasemble();
}


//------------------------------------------------------------------------------------------------
// Expression compiler
//------------------------------------------------------------------------------------------------
void VMCompiler::parseExpression() {
	Token tkn;
	parseTerm();
	tkn = parser->getToken(currentToken);
	while (tkn.type==TokenType::PLUS || tkn.type==TokenType::MINUS) {
		currentToken++;
		parseTerm();
		if (tkn.type == TokenType::PLUS) {
			destImage->emit(OP_ADD);
		} else {
			destImage->emit(OP_SUB);
		}
		tkn = parser->getToken(currentToken);
	}
}


void VMCompiler::parseTerm() {
	Token tkn;
	parseFactor();
	currentToken++;
	tkn = parser->getToken(currentToken);
	while (tkn.type == TokenType::MULTIPLY || tkn.type == TokenType::DIVIDE) {
		currentToken++;
		parseFactor();
		if (tkn.type == TokenType::MULTIPLY) {
			destImage->emit(OP_MUL);
		} else {
			destImage->emit(OP_DIV);
		}
		currentToken++;
		tkn = parser->getToken(currentToken);
	}

	
}

void VMCompiler::parseFactor() {
	Token tkn = parser->getToken(currentToken);
	bool unarMinus = false;
	if (tkn.type == TokenType::MINUS) {
		currentToken++;
		tkn = parser->getToken(currentToken);
		unarMinus = true;
	}
	if (tkn.type == TokenType::OP_PARENTHESES) {
		currentToken++;
		if (unarMinus) destImage->emit(OP_CONST, 0);
		parseExpression();
		if (unarMinus) destImage->emit(OP_SUB);
	} else if (tkn.type == TokenType::CONST_INTEGER) {
		if (unarMinus) destImage->emit(OP_CONST, 0);
		char buffer[32];
		strncpy(buffer, tkn.text, tkn.length);
		buffer[tkn.length] = 0;
		destImage->emit(OP_CONST, atoi(buffer));
		if (unarMinus) destImage->emit(OP_SUB);
	}
}


