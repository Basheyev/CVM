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


	currentToken = 0;
	root = parseBlock();



	return root;
}



VMNode* VMSyntaxTree::parseFunction() {
	return NULL;
}


VMNode* VMSyntaxTree::parseDeclaration() {

	// declaration :== TYPE identifier {= expression}
	return NULL;
}


VMNode* VMSyntaxTree::parseBlock() {
	VMNode* stmt, *block = NULL;
	Token token, prevToken;
	token = lexer->getToken(currentToken);
	// single statement
	if (token.type != TokenType::OP_BRACES)	return parseStatement();
	// block
	block = new VMNode({ TokenType::NONE, "BLOCK", 5, 0,0 });
	do {
		currentToken++;
		stmt = parseStatement();
		block->addChild(stmt);
		token = lexer->getToken(currentToken);
	} while (token.type != TokenType::CL_BRACES);
	currentToken++;
	return block;
}


VMNode* VMSyntaxTree::parseStatement() {
	Token token = lexer->getToken(currentToken);
	Token nextToken = lexer->getToken(currentToken + 1);

	switch (token.type) {
	case TokenType::IDENTIFIER:
		if (nextToken.type == TokenType::ASSIGN) return parseAssignment();
		break;
	case TokenType::IF:
		if (nextToken.type == TokenType::OP_PARENTHESES) {
			VMNode* ifblock = new VMNode(token);
			currentToken+=2; // skip 'if ('
			VMNode* condition = parseLogical();
			currentToken++;  // skip ')'
			VMNode* mainBlock = parseBlock();
			ifblock->addChild(condition);
			ifblock->addChild(mainBlock);
			token = lexer->getToken(currentToken);
			if (token.type == TokenType::ELSE) {
				currentToken++; // skip 'else'
				VMNode* elseBlock = parseBlock();
				ifblock->addChild(elseBlock);
			}
			return ifblock;
		}
		break;
	case TokenType::WHILE:
		if (nextToken.type == TokenType::OP_PARENTHESES) {
			VMNode* whileBlock = new VMNode(token);
			currentToken+=2; // skip 'while ('
			VMNode* condition = parseLogical();
			currentToken++;  // skip ')'
			VMNode* mainBlock = parseBlock();
			whileBlock->addChild(condition);
			whileBlock->addChild(mainBlock);
			return whileBlock;
		}
		break;
	}


	// call ()
	return NULL;
}


VMNode* VMSyntaxTree::parseAssignment() {
	VMNode* a, * b, * op = NULL;
	Token identifier = lexer->getToken(currentToken);
	Token assign = lexer->getToken(currentToken + 1);

	if (identifier.type == TokenType::IDENTIFIER && assign.type == TokenType::ASSIGN) {
		op = new VMNode(assign);
		a = new VMNode(identifier);
		currentToken += 2;
		b = parseLogical();
		op->addChild(a);
		op->addChild(b);
	}

	return op;
}



VMNode* VMSyntaxTree::parseLogical() {
	VMNode* a, * b, * op = NULL, * prevOp = NULL;
	a = parseExpression();
	Token token = lexer->getToken(currentToken);
	while (
		token.type == TokenType::EQUAL || 
		token.type == TokenType::NOT_EQUAL ||
		token.type == TokenType::GREATER ||
		token.type == TokenType::GR_EQUAL ||
		token.type == TokenType::LESS ||
		token.type == TokenType::LS_EQUAL) {
		currentToken++;
		b = parseExpression();
		op = new VMNode(token);
		if (prevOp == NULL) op->addChild(a); else op->addChild(prevOp);
		op->addChild(b);
		prevOp = op;
		token = lexer->getToken(currentToken);
	}
	return op == NULL ? a : op;
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
	} else if (token.type == TokenType::CONST_INTEGER || token.type == TokenType::IDENTIFIER) {
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

