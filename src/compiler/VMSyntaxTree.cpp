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

constexpr Token TKN_BLOCK = { TokenType::NONE, "BLOCK", 5, 0, 0 };
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

		root = parseModule();
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
// Tokens vector parsing recursive methods
//-----------------------------------------------------------------------------

VMNode* VMSyntaxTree::parseModule() {
	checkToken(TokenType::MODULE, "Module keyword expected");
	next();
	checkToken(TokenType::IDENTIFIER, "Module name expected");
	Token moduleName = getToken();
	next();
	checkToken(TokenType::EOS, "End of statement ';' expected");
	
	VMNode* moduleBlock = new VMNode(moduleName, VMNodeType::MODULE);
	VMNode* function;
	Token functionCheck;
	while (next()) {
		functionCheck = lexer->getToken(currentToken + 2);
		if (functionCheck.type == TokenType::OP_PARENTHESES) {
			moduleBlock->addChild(parseFunction());
		} else {
			moduleBlock->addChild(parseDeclaration());
		}
	}
	
	return moduleBlock;
}


//-----------------------------------------------------------------------------
// Parse declaration
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseDeclaration() {
	Token dataType = getToken();
	if (!isDataType(dataType.type)) raiseError("Variable data type expected");
	VMNode* variableDeclaration = new VMNode(dataType, VMNodeType::DATA_TYPE);
	while (next()) {
		if (isTokenType(TokenType::COMMA)) continue;
		if (isTokenType(TokenType::EOS)) break;
		checkToken(TokenType::IDENTIFIER, "Variable name expected");
		VMNode* variableName = new VMNode(getToken(), VMNodeType::SYMBOL);
		variableDeclaration->addChild(variableName);
	}
	return variableDeclaration;
}


//-----------------------------------------------------------------------------
// Parse function
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseFunction() {
	Token dataType = getToken();
	if (!isDataType(dataType.type)) raiseError("Function return data type expected");
	VMNode* returnType = new VMNode(dataType, VMNodeType::DATA_TYPE); next();
	checkToken(TokenType::IDENTIFIER, "Function name expected");
	VMNode* function = new VMNode(getToken(), VMNodeType::FUNCTION); next();
	VMNode* parameters = new VMNode({ TokenType::NONE, "PARAMETERS", 10, 0,0 }, VMNodeType::DECLARATION);
	while (next()) {
		if (isTokenType(TokenType::CL_PARENTHESES)) break;
		if (isTokenType(TokenType::COMMA)) continue;
		parameters->addChild(parseParameters());
	}
	next();
	VMNode* functionBody = parseBlock();

	function->addChild(returnType);
	function->addChild(parameters);
	function->addChild(functionBody);
	
	return function;
}


VMNode* VMSyntaxTree::parseParameters() {
	Token dataType = getToken();
	if (!isDataType(dataType.type)) raiseError("Funcation parameter data type expected");
	VMNode* parameterDeclaration = new VMNode(dataType, VMNodeType::DATA_TYPE); next();
	checkToken(TokenType::IDENTIFIER, "Function parameter name expected");
	VMNode* variableName = new VMNode(getToken(), VMNodeType::SYMBOL);
	parameterDeclaration->addChild(variableName);
	return parameterDeclaration;
}


//-----------------------------------------------------------------------------
// Parse block of statements
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseBlock() {
	if (!isTokenType(TokenType::OP_BRACES)) return parseStatement();
	VMNode* block = new VMNode(TKN_BLOCK, VMNodeType::BLOCK);
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
	if (isDataType(token.type)) return parseDeclaration(); else
	if (token.type == TokenType::IDENTIFIER) return parseAssignment(); else
	if (token.type == TokenType::IF) return parseIf(); else 
	if (token.type == TokenType::WHILE) return parseWhile(); else {
		raiseError("Unexpected token, statement expected");
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Parse if-else
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseIf() {
	VMNode* ifblock = new VMNode(getToken(), VMNodeType::IF_STATEMENT); next();
	checkToken(TokenType::OP_PARENTHESES, "Opening parentheses '(' expected");
	next();	ifblock->addChild(parseCondition());
	checkToken(TokenType::CL_PARENTHESES, "Closing parentheses ')' expected");
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
	VMNode* whileBlock = new VMNode(getToken(), VMNodeType::WHILE_STATEMENT); next();
	checkToken(TokenType::OP_PARENTHESES, "Opening parentheses '(' expected");
	next(); whileBlock->addChild(parseCondition());
	checkToken(TokenType::CL_PARENTHESES, "Closing parentheses ')' expected");
	next(); whileBlock->addChild(parseBlock());
	return whileBlock;
}


//-----------------------------------------------------------------------------
// Parse assignment
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseAssignment() {
	Token identifier = getToken(); next();
	checkToken(TokenType::ASSIGN, "Assignment operator '=' expected");
	VMNode* op = new VMNode(getToken(), VMNodeType::BINARY_OPERATION); next();
	VMNode* a = new VMNode(identifier, VMNodeType::SYMBOL);
	VMNode* b = parseCondition();
	op->addChild(a);
	op->addChild(b);
	return op;
}


//-----------------------------------------------------------------------------
// Parse condition
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseCondition() {
	VMNode* operand1, * operand2, * op = NULL, * prevOp = NULL;
	operand1 = parseExpression();
	Token token = getToken();
	while (isLogicOp(token.type)) {
		next();
		operand2 = parseExpression();
		op = new VMNode(token, VMNodeType::BINARY_OPERATION);
		if (prevOp == NULL) op->addChild(operand1); else op->addChild(prevOp);
		op->addChild(operand2);
		prevOp = op;
		token = getToken();
	}
	return op == NULL ? operand1 : op;
}


//-----------------------------------------------------------------------------
// Parse expression
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseExpression() {
	VMNode *operand1, *operand2, *op = NULL, *prevOp = NULL;
	operand1 = parseTerm();
	Token token = getToken();
	while (isTokenType(TokenType::PLUS) || isTokenType(TokenType::MINUS)) {
		next(); 
		operand2 = parseTerm();
		op = new VMNode(token, VMNodeType::BINARY_OPERATION);
		if (prevOp == NULL) op->addChild(operand1); else op->addChild(prevOp);
		op->addChild(operand2);
		prevOp = op;
		token = getToken();
	}
	return op == NULL ? operand1 : op;
}

//-----------------------------------------------------------------------------
// Parse term
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseTerm() {
	VMNode *operand1, *operand2, *op = NULL, *prevOp = NULL;
	operand1 = parseFactor();
	Token token = getToken();
	while (isTokenType(TokenType::MULTIPLY) || isTokenType(TokenType::DIVIDE)) {
		next();
		operand2 = parseFactor();
		op = new VMNode(token, VMNodeType::BINARY_OPERATION);
		if (prevOp == NULL) op->addChild(operand1); else op->addChild(prevOp);
		op->addChild(operand2);
		prevOp = op;
		token = lexer->getToken(currentToken);
	}
	return op == NULL ? operand1 : op;
}

//-----------------------------------------------------------------------------
// Parse factor
//-----------------------------------------------------------------------------
VMNode* VMSyntaxTree::parseFactor() {
	VMNode* factor = NULL;	
	bool unaryMinus = false;

	if (isTokenType(TokenType::MINUS)) { unaryMinus = true; next(); } else
	if (isTokenType(TokenType::PLUS)) { unaryMinus = false; next(); }
	else {
	
	
	}
	
	if (isTokenType(TokenType::OP_PARENTHESES)) {
		next();
		factor = parseExpression();
		Token token = getToken();
		if (isTokenType(TokenType::CL_PARENTHESES)) next(); 
		else raiseError("Closing parentheses expected");
	} else if (isTokenType(TokenType::CONST_INTEGER)) { 
		factor = new VMNode(getToken(), VMNodeType::CONSTANT); next(); 
	} else if (isTokenType(TokenType::IDENTIFIER)) { 
		factor = new VMNode(getToken(), VMNodeType::SYMBOL); next(); 
	} 
	else raiseError("Number or identifier expected");

	if (unaryMinus) {
		Token token = getToken();
		VMNode* expr = new VMNode(TKN_MINUS, VMNodeType::BINARY_OPERATION);
		VMNode* zero = new VMNode(TKN_ZERO, VMNodeType::CONSTANT);
		expr->addChild(zero);
		expr->addChild(factor);
		return expr;
	}

	return factor;
}

