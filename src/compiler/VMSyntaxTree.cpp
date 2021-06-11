/*============================================================================
*
*  Abstract Syntax Tree class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/

#include "compiler/VMSyntaxTree.h"


using namespace vm;
using namespace std;

VMSyntaxTree::VMSyntaxTree() {
	lexer = new VMLexer();
	root = NULL;
	currentToken = 0;
}

VMSyntaxTree::~VMSyntaxTree() {
	delete root;
	delete lexer;
}


VMNode* VMSyntaxTree::parse(const char* source) {
	
	if (root != NULL) {
		root->removeAll();
		root = NULL;
	}

	lexer->parseToTokens(source);

	try {
		currentToken = 0;
		root = parseExpression();
	} catch (Token token) {

		root->removeAll();
		root = NULL;
	}


	return root;
}



VMNode* VMSyntaxTree::parseFunction() {
	return NULL;
}


VMNode* VMSyntaxTree::parseDeclaration() {
	return NULL;
}


VMNode* VMSyntaxTree::parseStatement() {
	return NULL;
}


VMNode* VMSyntaxTree::parseAssignment() {
	return NULL;
}


VMNode* VMSyntaxTree::parseExpression() {
	VMNode *a, *b, *op = NULL, *prevOp = NULL;
	a = parseTerm();
	Token token = lexer->getToken(currentToken);
	while (token.type == TokenType::PLUS || token.type == TokenType::MINUS) {
		currentToken++;                           
		b = parseTerm();
		op = new VMNode(token);
		if (prevOp == NULL) op->addChild(a); else op->addChild(prevOp);
		op->addChild(b);
		prevOp = op;
		token = lexer->getToken(currentToken);
	}
	return op == NULL ? a : op;
}


VMNode* VMSyntaxTree::parseTerm() {
	VMNode* a, * b, * op = NULL, * prevOp = NULL;
	a = parseFactor();
	Token token = lexer->getToken(currentToken);
	while (token.type == TokenType::MULTIPLY || token.type == TokenType::DIVIDE) {
		currentToken++;
		b = parseFactor();
		op = new VMNode(token);
		if (prevOp == NULL) op->addChild(a); else op->addChild(prevOp);
		op->addChild(b);
		prevOp = op;
		token = lexer->getToken(currentToken);
	}
	return op == NULL ? a : op;
}


VMNode* VMSyntaxTree::parseFactor() {
	VMNode* a = NULL, *b = NULL, *op = NULL;
	Token token = lexer->getToken(currentToken);
	bool unaryMinus = false;

	if (token.type == TokenType::MINUS) {
		currentToken++;
		token = lexer->getToken(currentToken);
		unaryMinus = true;
	}

	if (token.type == TokenType::OP_PARENTHESES) {
		currentToken++;
		b = parseExpression();
		currentToken++;
	} else if (token.type == TokenType::CONST_INTEGER) {
		b = new VMNode(token);
		currentToken++;
	} else {
		// throw exception
		return new VMNode(token);
	}

	if (unaryMinus) {
		a = new VMNode({ TokenType::CONST_INTEGER, "0", 1, token.row, token.col});
		op = new VMNode({ TokenType::MINUS, "-", 1, token.row, token.col});
		op->addChild(a);
		op->addChild(b);
		return op;
	}

	return b;
}

