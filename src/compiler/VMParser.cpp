/*============================================================================
*
*  Abstract Syntax Tree class implementation
*
*  (C) Bolat Basheyev 2021
*  
*  C like simplified language grammar - define language grammar:
* 
*  <module>          ::= 'module' <identifier> {<declaration>|<function>}*
*  <data-type>       ::= 'int'
*  <declaration>     ::= <data-type> <identifier> {','<identifier>}* ';' 
*  <function>        ::= <data-type> <identifier> '(' <argument> {, <argument>}* ')' <block>
*  <argument>        ::= <data-type> <identifier>
*  <statement>       ::= <block> | <declration> | <assignment> | <if-statement> | <while-statement> | <jump-statement> | <call>)
*  <call>            ::= <identifier> '(' {<expression>} {, expression}* ')'
*  <block>           ::= '{' {<statement>}* '}'
*  <if-statement>    ::= 'if' '(' <expression> ')' <statement>
*  <while-statement> ::= 'while' '(' <expression> ')' <statement>
*  <jump-statement>  ::= 'return' <expression> ';'
*  <assignment>      ::= <identifier> = <expression> ';'
*  <expression>      ::= <term> {(+|-) <term>}
*  <term>            ::= <factor> {(*|/) <factor>}
*  <factor>          ::= ({-|+} <number>) | <identifer> | <call>
* 
============================================================================*/

#include "compiler/VMParser.h"
#include <iostream>

using namespace vm;
using namespace std;

constexpr Token TKN_ARGUMENTS = { TokenType::NONE, "ARGUMENTS", 9, 0, 0 };
constexpr Token TKN_BLOCK = { TokenType::NONE, "BLOCK", 5, 0, 0 };
constexpr Token TKN_ZERO = { TokenType::CONST_INTEGER, "0", 1, 0, 0 };
constexpr Token TKN_MINUS = { TokenType::MINUS, "-", 1, 0, 0 };

VMParser::VMParser() {
	lexer = new VMLexer();
	root = NULL;
	currentToken = 0;
}

VMParser::~VMParser() {
	delete root;
	delete lexer;
}


VMNode* VMParser::parse(const char* source) {
	
	if (root != NULL) {
		root->removeAll();
		root = NULL;
	}

	currentToken = 0;
	lexer->parseToTokens(source);
	//lexer->printAllTokens();

	try {

		root = parseModule();
	}
	catch (ParserException& e) {
		// FIXME nodes memory leakage when exception thrown
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
// Rule:
// <module> ::= 'module' <identifier> {<declaration>|<function>}*
//-----------------------------------------------------------------------------
VMNode* VMParser::parseModule() {
	checkToken(TokenType::MODULE, "Module keyword expected");
	next();
	checkToken(TokenType::IDENTIFIER, "Module name expected");
	Token moduleName = getToken();
	next();
	checkToken(TokenType::EOS, "End of statement ';' expected");
	
	VMNode* moduleBlock = new VMNode(moduleName, VMNodeType::MODULE);
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
// Rule:
// <declaration> ::= <data-type> <identifier> {','<identifier>}* ';' 
//-----------------------------------------------------------------------------
VMNode* VMParser::parseDeclaration() {
	Token dataType = getToken();
	if (!isDataType(dataType.type)) raiseError("Data type expected");
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
// Rule:
// <function> ::= <data-type> <identifier> '(' <argument> {, <argument>}* ')' <block>
//-----------------------------------------------------------------------------
VMNode* VMParser::parseFunction() {
	Token dataType = getToken();
	if (!isDataType(dataType.type)) raiseError("Function return data type expected");
	VMNode* returnType = new VMNode(dataType, VMNodeType::DATA_TYPE); next();
	checkToken(TokenType::IDENTIFIER, "Function name expected");
	VMNode* function = new VMNode(getToken(), VMNodeType::FUNCTION); next();
	VMNode* parameters = new VMNode(TKN_ARGUMENTS, VMNodeType::DECLARATION);
	while (next()) {
		if (isTokenType(TokenType::CL_PARENTHESES)) break;
		if (isTokenType(TokenType::COMMA)) continue;
		parameters->addChild(parseArguments());
	}
	next();
	VMNode* functionBody = parseBlock();

	function->addChild(returnType);
	function->addChild(parameters);
	function->addChild(functionBody);
	
	return function;
}

//-----------------------------------------------------------------------------
// Rule:
// <argument> ::= <data-type> <identifier>
//-----------------------------------------------------------------------------
VMNode* VMParser::parseArguments() {
	Token dataType = getToken();
	if (!isDataType(dataType.type)) raiseError("Funcation parameter data type expected");
	VMNode* parameterDeclaration = new VMNode(dataType, VMNodeType::DATA_TYPE); next();
	checkToken(TokenType::IDENTIFIER, "Function parameter name expected");
	VMNode* variableName = new VMNode(getToken(), VMNodeType::SYMBOL);
	parameterDeclaration->addChild(variableName);
	return parameterDeclaration;
}

//-----------------------------------------------------------------------------
// Rule:
// <call> ::= <identifier> '(' {<expression>} {, expression}* ')'
//-----------------------------------------------------------------------------
VMNode* VMParser::parseCall() {
	Token identifier = getToken();
	VMNode* callNode = new VMNode(identifier, VMNodeType::CALL); next();
	if (!isTokenType(TokenType::OP_PARENTHESES)) raiseError("Opening parentheses '(' expected.");
	while (next()) {
		Token tkn = getToken();
		if (isTokenType(TokenType::CL_PARENTHESES)) break;
		if (isTokenType(TokenType::COMMA)) continue;
		VMNode* param = parseExpression();
		callNode->addChild(param);
		if (isTokenType(TokenType::CL_PARENTHESES)) break;
	}
	return callNode;
}


//-----------------------------------------------------------------------------
// Rule:
// <block> ::= '{' {<statement>}* '}'
//-----------------------------------------------------------------------------
VMNode* VMParser::parseBlock() {
	//if (!isTokenType(TokenType::OP_BRACES)) return parseStatement();
	VMNode* block = new VMNode(TKN_BLOCK, VMNodeType::BLOCK);
	while (next()) {
		if (isTokenType(TokenType::CL_BRACES)) break;
		if (isTokenType(TokenType::EOS)) continue;
		block->addChild(parseStatement());
	}
	return block;
}


//-----------------------------------------------------------------------------
// Rule:
// <statement> ::= <block> | <declaration> | <call> | <assignment> | <if-statement> | <while-statement> | <jump-statement> 
// <jump-statement>  ::= 'return' <expression> ';'
//-----------------------------------------------------------------------------
VMNode* VMParser::parseStatement() {
	Token token = getToken();
	if (isDataType(token.type)) return parseDeclaration(); else
	if (token.type == TokenType::OP_BRACES) return parseBlock(); else
	if (token.type == TokenType::IDENTIFIER) {
		Token nextToken = getNextToken();
		if (nextToken.type == TokenType::ASSIGN) return parseAssignment();
		if (nextToken.type == TokenType::OP_PARENTHESES) {
			VMNode* callNode = parseCall(); next();
			if (!isTokenType(TokenType::EOS)) raiseError("';' expected");
			return callNode;
		} else {
			raiseError("Unexpected token, assignment '=' or function call expecated.");
		}
	} 
	else
	if (token.type == TokenType::IF) return parseIf(); else 
	if (token.type == TokenType::WHILE) return parseWhile(); else
	if (token.type == TokenType::RETURN) {
		VMNode* returnStmt = new VMNode(token, VMNodeType::RETURN); next();
		VMNode* expr = parseExpression();
		returnStmt->addChild(expr);
		return returnStmt;
	}
	else {
		raiseError("Unexpected token, statement expected");
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Parse if-else
//-----------------------------------------------------------------------------
VMNode* VMParser::parseIf() {
	VMNode* ifblock = new VMNode(getToken(), VMNodeType::IF_STATEMENT); next();
	checkToken(TokenType::OP_PARENTHESES, "Opening parentheses '(' expected");
	next();	ifblock->addChild(parseCondition());
	checkToken(TokenType::CL_PARENTHESES, "Closing parentheses ')' expected");
	next();	ifblock->addChild(parseStatement());
	if (getNextToken().type==TokenType::ELSE) {
		next(); next(); // FIXME why 2 next???
		ifblock->addChild(parseStatement());
	}
	return ifblock;
}


//-----------------------------------------------------------------------------
// Parse while 
//-----------------------------------------------------------------------------
VMNode* VMParser::parseWhile() {
	VMNode* whileBlock = new VMNode(getToken(), VMNodeType::WHILE_STATEMENT); next();
	checkToken(TokenType::OP_PARENTHESES, "Opening parentheses '(' expected");
	next(); whileBlock->addChild(parseCondition());
	checkToken(TokenType::CL_PARENTHESES, "Closing parentheses ')' expected");
	next(); whileBlock->addChild(parseStatement());
	return whileBlock;
}


//-----------------------------------------------------------------------------
// Parse assignment
//-----------------------------------------------------------------------------
VMNode* VMParser::parseAssignment() {
	Token identifier = getToken(); next();
	checkToken(TokenType::ASSIGN, "Assignment operator '=' expected");
	VMNode* op = new VMNode(getToken(), VMNodeType::ASSIGNMENT); next();
	VMNode* a = new VMNode(identifier, VMNodeType::SYMBOL);
	VMNode* b = parseCondition();
	op->addChild(a);
	op->addChild(b);
	return op;
}


//-----------------------------------------------------------------------------
// Parse condition
//-----------------------------------------------------------------------------
VMNode* VMParser::parseCondition() {
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
VMNode* VMParser::parseExpression() {
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
VMNode* VMParser::parseTerm() {
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
VMNode* VMParser::parseFactor() {
	VMNode* factor = NULL;	
	bool unaryMinus = false;

	if (isTokenType(TokenType::MINUS)) { unaryMinus = true; next(); } else
	if (isTokenType(TokenType::PLUS)) { unaryMinus = false; next(); }
	else {
		// add unary Not operator	
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
		Token nextToken = getNextToken();
		if (nextToken.type == TokenType::OP_PARENTHESES) {
			factor = parseCall(); next();
		} else {
			factor = new VMNode(getToken(), VMNodeType::SYMBOL); next();
		}
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

