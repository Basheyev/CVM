/*============================================================================
*
*  Abstract Syntax Tree class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/

#include "compiler/VMSyntaxTree.h"
#include <iostream>

using namespace vm;
using namespace std;

constexpr Token TKN_ZERO = { TokenType::CONST_INTEGER, "0", 1, 0, 0 };
constexpr Token TKN_MINUS = { TokenType::MINUS, "-", 1, 0, 0 };

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

	currentToken = 0;
	lexer->parseToTokens(source);

	try {
		root = parseFunction();
	}
	catch (ParserException& e) {
		// nodes memory leakage
		cerr << "ParserException: " << e.msg << endl;
		cerr << "Token: ";
		cerr.write(e.token.text, e.token.length);
		cout << endl;
		cerr << "Line: " << e.token.row << " Col: " << e.token.col << endl;
		return NULL;
	}

	return root;
}


//-----------------------------------------------------------------------------
// Parsing recursive methods
//-----------------------------------------------------------------------------

VMNode* VMSyntaxTree::parseFunction() {
	return parseBlock();
}


VMNode* VMSyntaxTree::parseDeclaration() {
	// declaration :== TYPE identifier {= expression}
	return NULL;
}


//-----------------------------------------------------------------------------
// Parse block of statements
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseBlock() {
	if (!isTokenType(TokenType::OP_BRACES)) return parseStatement();
	VMNode* block = new VMNode({ TokenType::NONE, "BLOCK", 5, 0,0 });
	while (next()) {
		if (isTokenType(TokenType::CL_BRACES)) break;
		if (isTokenType(TokenType::EOS)) continue;
		block->addChild(parseStatement());
	}
	return block;
}

//-----------------------------------------------------------------------------
// Parse statement
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseStatement() {
	Token token = getToken();
	switch (token.type) {
		case TokenType::IDENTIFIER: return parseAssignment();
		case TokenType::IF: return parseIf();
		case TokenType::WHILE: return parseWhile();
		default: raiseError("Unexpected token, statement expected");
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Parse if-else
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseIf() {
	VMNode* ifblock = new VMNode(getToken()); next();
	check(TokenType::OP_PARENTHESES, "Opening parentheses '(' expected");
	next();	ifblock->addChild(parseCondition());
	check(TokenType::CL_PARENTHESES, "Closing parentheses ')' expected");
	next();	ifblock->addChild(parseBlock());
	if (getNextToken().type==TokenType::ELSE) {
		next(); next(); // FIXME why 2 next???
		ifblock->addChild(parseBlock());
	}
	return ifblock;
}


//-----------------------------------------------------------------------------
// Parse while 
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseWhile() {
	VMNode* whileBlock = new VMNode(getToken()); next();
	check(TokenType::OP_PARENTHESES, "Opening parentheses '(' expected");
	next(); whileBlock->addChild(parseCondition());
	check(TokenType::CL_PARENTHESES, "Closing parentheses ')' expected");
	next(); whileBlock->addChild(parseBlock());
	return whileBlock;
}


//-----------------------------------------------------------------------------
// Parse assignment
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseAssignment() {
	Token identifier = getToken(); next();
	check(TokenType::ASSIGN, "Assignment '=' expected");
	VMNode* op = new VMNode(getToken()); next();
	VMNode* a = new VMNode(identifier);
	VMNode* b = parseCondition();
	op->addChild(a);
	op->addChild(b);
	return op;
}


//-----------------------------------------------------------------------------
// Parse condition
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseCondition() {
	VMNode* a, * b, * op = NULL, * prevOp = NULL;
	a = parseExpression();
	Token token = getToken();
	while (isLogicOp(token.type)) {
		next();
		b = parseExpression();
		op = new VMNode(token);
		if (prevOp == NULL) op->addChild(a); else op->addChild(prevOp);
		op->addChild(b);
		prevOp = op;
		token = getToken();
	}
	return op == NULL ? a : op;
}


//-----------------------------------------------------------------------------
// Parse expression
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseExpression() {
	VMNode *a, *b, *op = NULL, *prevOp = NULL;
	a = parseTerm();
	Token token = getToken();
	while (isTokenType(TokenType::PLUS) || isTokenType(TokenType::MINUS)) {
		next(); 
		b = parseTerm();
		op = new VMNode(token);
		if (prevOp == NULL) op->addChild(a); else op->addChild(prevOp);
		op->addChild(b);
		prevOp = op;
		token = getToken();
	}
	return op == NULL ? a : op;
}

//-----------------------------------------------------------------------------
// Parse term
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseTerm() {
	VMNode* a, * b, * op = NULL, * prevOp = NULL;
	a = parseFactor();
	Token token = getToken();
	while (isTokenType(TokenType::MULTIPLY) || isTokenType(TokenType::DIVIDE)) {
		next();
		b = parseFactor();
		op = new VMNode(token);
		if (prevOp == NULL) op->addChild(a); else op->addChild(prevOp);
		op->addChild(b);
		prevOp = op;
		token = lexer->getToken(currentToken);
	}
	return op == NULL ? a : op;
}

//-----------------------------------------------------------------------------
// Parse factor
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseFactor() {
	VMNode* factor = NULL;	
	bool unaryMinus = false;

	if (isTokenType(TokenType::MINUS)) { unaryMinus = true; next(); }

	if (isTokenType(TokenType::OP_PARENTHESES)) {
		next();
		factor = parseExpression();
		Token token = getToken();
		if (isTokenType(TokenType::CL_PARENTHESES)) next(); 
		else raiseError("Closing parentheses expected");
	} 
	else if (isTokenType(TokenType::CONST_INTEGER)) { factor = new VMNode(getToken());	next(); }
	else if (isTokenType(TokenType::IDENTIFIER)) { factor = new VMNode(getToken()); next(); } 
	else raiseError("Number or identifier expected");

	if (unaryMinus) {
		Token token = getToken();
		VMNode* zero = new VMNode(TKN_ZERO);
		VMNode* expr = new VMNode(TKN_MINUS);
		expr->addChild(zero);
		expr->addChild(factor);
		return expr;
	}

	return factor;
}

