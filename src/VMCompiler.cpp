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
	parseExpression(0, destImage);
	destImage->dissasemble();
}


//------------------------------------------------------------------------------------------------
// Expression compiler
//------------------------------------------------------------------------------------------------
void VMCompiler::parseExpression(size_t startIndex, VMImage* destImage) {
	Token tkn;
	currentToken = startIndex;
	parseTerm(destImage);
	tkn = parser->getToken(currentToken);
	while (tkn.type==TokenType::PLUS || tkn.type==TokenType::MINUS) {
		currentToken++;
		parseTerm(destImage);
		if (tkn.type == TokenType::PLUS) {
			destImage->emit(OP_ADD);
		}
		else {
			destImage->emit(OP_SUB);
		}
		tkn = parser->getToken(currentToken);
	}
}


void VMCompiler::parseTerm(VMImage* destImage) {
	Token tkn;
	parseFactor(destImage);
	currentToken++;
	tkn = parser->getToken(currentToken);
	while (tkn.type == TokenType::MULTIPLY || tkn.type == TokenType::DIVIDE) {
		currentToken++;
		parseFactor(destImage);
		if (tkn.type == TokenType::MULTIPLY) {
			destImage->emit(OP_MUL);
		}
		else {
			destImage->emit(OP_DIV);
		}
		currentToken++;
		tkn = parser->getToken(currentToken);
	}

	
}

void VMCompiler::parseFactor(VMImage* destImage) {
	Token tkn = parser->getToken(currentToken);
	char buffer[32];
	strncpy(buffer, tkn.text, tkn.length);
	buffer[tkn.length] = 0;
	destImage->emit(OP_CONST, atoi(buffer));
}


