/*============================================================================
*
*  Virtual Machine Compiler Source Code Parser implementation
*
*  Basic C like language grammar:
*
*  <module>      ::= {<declaration>|<function>}*
*  <type>        ::= 'int'
*  <declaration> ::= <type> <identifier> {','<identifier>}* ';'
*  <function>    ::= <type> <identifier> '(' <argument> {, <argument>}* ')' <block>
*  <argument>    ::= <type> <identifier>
*  <statement>   ::= <block> | <declaration> | <assign> | <if-else> | <while> | <jump> | <call>
*  <block>       ::= '{' {<statement>}* '}'
*  <call>        ::= <identifier> '(' {<expression>} {, expression}* ')'
*  <if-else>     ::= 'if' '(' <condition> ')' <statement> { 'else' <statement> }
*  <while>       ::= 'while' '(' <condition> ')' <statement>
*  <jump>        ::= 'return' <expression> ';'
*  <assign>      ::= <identifier> = <expression> ';'
*  <condition>   ::= <expression> {( == | != | > | >= | < | <= | && | '||') <expression>}
*  <expression>  ::= <term> {(+|-) <term>}
*  <term>        ::= <bitwise> {(*|/) <bitwise>}
*  <bitwise>     ::= <factor> {( & | '|' | ^ | ~ | << | >> ) <factor>}
*  <factor>      ::= ({!|-|+} <number>) | <identifer> | <call>
*
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include <iostream>
#include "compiler/SourceParser.h"

using namespace vm;


//-----------------------------------------------------------------------------
// Constructor - builds source code abstract syntax tree
//-----------------------------------------------------------------------------
SourceParser::SourceParser(const char* sourceCode) {
    try {
        parseToTokens(sourceCode);
        buildSyntaxTree();
    }
    catch (ParserException e) {
        TokenType type = e.token.type;
        cout << "PARSER EXCEPTION: " << e.msg << endl;
        cout << "Token at row=" << e.token.row << " col=" << e.token.col;
        if (type == TokenType::IDENTIFIER || 
            type == TokenType::CONST_INTEGER || 
            type == TokenType::CONST_STRING) {
            cout << " '";
            cout.write(e.token.text, e.token.length);
            cout << "'" << endl;
        }
        else cout << "  '" << TOKEN_TYPE_MNEMONIC[(int)type] << "'" << endl;
    }
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
SourceParser::~SourceParser() {
    if (root != NULL) delete root;
}


//-----------------------------------------------------------------------------
// Parses source code to tokens
//-----------------------------------------------------------------------------
void SourceParser::parseToTokens(const char* sourceCode) {

    tokens.clear();                                                    // clear tokens vector
    int length;                                                        // token length variable
    int row = 1, col = 1;                                              // reset current row, col counter
    char* cursor = (char*)sourceCode;                                  // set cursor to source beginning
    char* newLine = cursor;                                            // new line pointer
    char* start = cursor;                                              // start new token from cursor
    char value = *cursor;                                              // read first char from cursor
    char nextChar;                                                     // next char variable
    bool insideString = false;                                         // inside string flag

    while (value != NULL) {                                            // while not end of string (NULL)
        length = (int) (cursor - start);                               // measure token length
        if ((isBlank(value) || isDelimeter(value)) && !insideString) { // if there is token separator  
            if (value == '\n') {                                       // if new line '\n' found 
                row++; col = 1;                                        // increment row, reset col
                newLine = cursor + 1;                                  // set new line pointer
            }
            if (length > 0) pushToken(start, length, row, col);        // if length > 0 push token to vector
            nextChar = cursor[1];                                      // get next char after cursor
            if (isDelimeter(value) && isDelimeter(nextChar)) {         // if next char is also delimeter
                if (!pushToken(cursor, 2, row, col))                   // try to push double char delimeter token
                    pushToken(cursor, 1, row, col);                    // if not pushed - its single char delimeter
                else cursor++;                                         // if double delimeter, increment cursor
            } else pushToken(cursor, 1, row, col);                     // else push single char delimeter
            start = cursor + 1;                                        // calculate next token start pointer
            col = (int) (start - newLine + 1);                         // calculate token start column
        }
        else if (value == '"') insideString = !insideString;           // if '"' char - flip insideString flag 
        else if (insideString && value == '\n') {                      // if '\n' found inside string
            Token tkn{TokenType::UNKNOWN,start,length,row,col};        // use token information
            raiseError(tkn, "Can't use '\\n' in in string constant."); // and throw exception
        }
        cursor++;                                                      // increment cursor pointer
        value = *cursor;                                               // read next char
    }

    length = (int) (cursor - start);                                   // if there is a last token
    if (length > 0) pushToken(start, length, row, col);                // push last token to vector
}


//-----------------------------------------------------------------------------
// Pushes parsed token to tokens vector
//-----------------------------------------------------------------------------
bool SourceParser::pushToken(char* text, int length, int row, int col) {
    TokenType type = getTokenType(text, length);
    if (type != TokenType::UNKNOWN) {
        tokens.push_back({ type, text, length, row, col });
        return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
// Identifies token type by comparing to keywords, operators and rules
//-----------------------------------------------------------------------------
TokenType SourceParser::getTokenType(char* text, int length) {
    if (text == NULL || length < 1) return TokenType::UNKNOWN;
    for (int i = 0; i < TOKEN_TYPE_COUNT; i++) {
        const char* mnemonic = TOKEN_TYPE_MNEMONIC[i];
        int mnemonicLength = (int) strlen(mnemonic);
        if (length == mnemonicLength) {
            if (strncmp(text, mnemonic, mnemonicLength) == 0) {
                return (TokenType) i;
            }
        }
    }
    char firstChar = text[0];
    if (isdigit(firstChar)) return validateNumber(text, length);
    if (isalpha(firstChar)) return validateIdentifier(text, length);
    if (firstChar == '"') return validateString(text, length);
    return TokenType::UNKNOWN;
}


//-----------------------------------------------------------------------------
// Validates constant integer number
//-----------------------------------------------------------------------------
TokenType SourceParser::validateNumber(char* text, int length) {
    for (size_t i = 1; i < length; i++)
        if (!isdigit(text[i])) return TokenType::UNKNOWN;
    return TokenType::CONST_INTEGER;
}


//-----------------------------------------------------------------------------
// Validates identifier
//-----------------------------------------------------------------------------
TokenType SourceParser::validateIdentifier(char* text, int length) {
    for (size_t i = 1; i < length; i++)
        if (!isalnum(text[i])) return TokenType::UNKNOWN;
    return TokenType::IDENTIFIER;
}


//-----------------------------------------------------------------------------
// Validates constant string
//-----------------------------------------------------------------------------
TokenType SourceParser::validateString(char* text, int length) {
    if (length < 2) return TokenType::UNKNOWN;
    if (text[length - 1] != '"') return TokenType::UNKNOWN;
    return TokenType::CONST_STRING;
}


//---------------------------------------------------------------------------
// Builds abstract syntax tree
//---------------------------------------------------------------------------
void SourceParser::buildSyntaxTree() {
    currentToken = 0;

    // add iput system function
    Token iput = { TokenType::IDENTIFIER, "iput", 4, 0,0 };
    rootSymbolTable.addSymbol(iput, SymbolType::FUNCTION);
    
    root = parseModule(&rootSymbolTable);
}


//---------------------------------------------------------------------------
// <module> ::= { <declaration> | <function> }*
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseModule(SymbolTable* scope) {
    TreeNode* program = new TreeNode(EMPTY_TOKEN, TreeNodeType::MODULE, scope);
    Token functionCheck;
    do {
        functionCheck = getToken(currentToken + 2);
        if (functionCheck.type == TokenType::OP_PARENTHESES) {
            program->addChild(parseFunction(scope));
        } else {
            program->addChild(parseDeclaration(scope));
        }
    } while (next());
    return program; 
}


//---------------------------------------------------------------------------
// <declaration> ::= <type> <identifier> {','<identifier>}* ';'
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseDeclaration(SymbolTable* scope) {
    Token dataType = getToken();
    if (!isDataType(dataType.type)) raiseError("Data type expected");
    TreeNode* variableDeclaration = new TreeNode(dataType, TreeNodeType::TYPE, scope);
    while (next()) {
        if (isTokenType(TokenType::COMMA)) next(); else
        if (isTokenType(TokenType::EOS)) break;
        checkToken(TokenType::IDENTIFIER, "Variable name expected");
        TreeNode* variableName = new TreeNode(getToken(), TreeNodeType::SYMBOL, scope);
        if (!scope->addSymbol(variableName->getToken(), SymbolType::VARIABLE)) {
            raiseError("Variable already defined.");
        }
        variableDeclaration->addChild(variableName);
    }
    return variableDeclaration;
}


//---------------------------------------------------------------------------
// <function> ::= <type> <identifier> '(' <argument> {, <argument>}* ')' <block>
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseFunction(SymbolTable* scope) {
    Token dataType = getToken();
    if (!isDataType(dataType.type)) raiseError("Function return data type expected");
    TreeNode* returnType = new TreeNode(dataType, TreeNodeType::TYPE, scope); next();
    checkToken(TokenType::IDENTIFIER, "Function name expected");
    TreeNode* function = new TreeNode(getToken(), TreeNodeType::FUNCTION, scope); 
    if (!scope->addSymbol(function->getToken(), SymbolType::FUNCTION)) {
        raiseError("Function already defined.");
    } else next();

    string functionName;
    functionName.append(function->getToken().text, function->getToken().length);
    SymbolTable* blockSymbols = new SymbolTable(functionName);
    scope->addChild(blockSymbols);
    TreeNode* arguments = new TreeNode(EMPTY_TOKEN, TreeNodeType::SYMBOL, blockSymbols);
    while (next()) {
        Token tkn = getToken();
        if (isTokenType(TokenType::COMMA)) next(); else
        if (isTokenType(TokenType::CL_PARENTHESES)) break; 
        arguments->addChild(parseArgument(blockSymbols));
    }
    next();

    TreeNode* functionBody = parseBlock(blockSymbols, true);
    function->addChild(returnType);
    function->addChild(arguments);
    function->addChild(functionBody);
    return function;
}


//---------------------------------------------------------------------------
// <argument> :: = <type> <identifier>
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseArgument(SymbolTable* scope) {
    Token dataType = getToken();
    if (!isDataType(dataType.type)) raiseError("Function argument type expected");
    TreeNode* argument = new TreeNode(dataType, TreeNodeType::TYPE, scope); next();
    checkToken(TokenType::IDENTIFIER, "Function argument name expected");
    TreeNode* variableName = new TreeNode(getToken(), TreeNodeType::SYMBOL, scope);
    if (!scope->addSymbol(variableName->getToken(), SymbolType::ARGUMENT)) {
        raiseError("Argument already defined.");
    }
    argument->addChild(variableName);
    return argument;
}



//---------------------------------------------------------------------------
// <block> ::= '{' {<statement>}* '}'
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseBlock(SymbolTable* scope, bool isFunction) {
    TreeNode* block = new TreeNode(TKN_BLOCK, TreeNodeType::BLOCK, scope);
    SymbolTable* blockSymbols;
    if (isFunction) blockSymbols = scope; else {
        string name = "block";
        name.append(to_string(blockCounter++));
        blockSymbols = new SymbolTable(name);
        scope->addChild(blockSymbols);
        block->setSymbolTable(blockSymbols);
    }
    while (next()) {
        if (isTokenType(TokenType::CL_BRACES)) break;
        if (isTokenType(TokenType::EOS)) continue;
        block->addChild(parseStatement(blockSymbols));
    }
    // todo check logic
    if (blockSymbols->getSymbolsCount() == 0) {
     //   scope->removeChild(blockSymbols);
      //  block->setSymbolTable(scope);
    }
    return block;
}



//---------------------------------------------------------------------------
// <statement> ::= <block> | <declration> | <assign> | <if-else> | <while> | <jump> | <call>
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseStatement(SymbolTable* scope) {
    Token token = getToken();
    if (isDataType(token.type)) return parseDeclaration(scope); else
    if (token.type == TokenType::OP_BRACES) return parseBlock(scope, false); else
    if (token.type == TokenType::IDENTIFIER) {
        Token nextToken = getNextToken();
        if (nextToken.type == TokenType::ASSIGN) return parseAssignment(scope);
        if (nextToken.type == TokenType::OP_PARENTHESES) {
            TreeNode* callNode = parseCall(scope); next();
            if (!isTokenType(TokenType::EOS)) raiseError("';' expected");
            return callNode;
        } else raiseError("Unexpected token, assignment '=' or function call expecated.");
    } else
    if (token.type == TokenType::IF) return parseIfElse(scope); else
    if (token.type == TokenType::WHILE) return parseWhile(scope); else
    if (token.type == TokenType::RETURN) {
        TreeNode* returnStmt = new TreeNode(token, TreeNodeType::RETURN, scope); next();
        TreeNode* expr = parseExpression(scope);
        returnStmt->addChild(expr);
        return returnStmt;
    } else raiseError("Unexpected token, statement expected");
    return NULL;
}


//---------------------------------------------------------------------------
// <call> ::= <identifier> '(' {<expression>} {, expression}* ')'
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseCall(SymbolTable* scope) {
    Token identifier = getToken();
    if (scope->lookupSymbol(identifier) == NULL) {
        raiseError("Symbol not defined.");
    }
    TreeNode* callNode = new TreeNode(identifier, TreeNodeType::CALL, scope); next();
    if (!isTokenType(TokenType::OP_PARENTHESES)) raiseError("Opening parentheses '(' expected.");
    while (next()) {
        Token tkn = getToken();
        if (isTokenType(TokenType::CL_PARENTHESES)) break;
        if (isTokenType(TokenType::COMMA)) continue;
        TreeNode* param = parseExpression(scope);
        callNode->addChild(param);
        if (isTokenType(TokenType::CL_PARENTHESES)) break;
    }
    return callNode;
}


//---------------------------------------------------------------------------
// <if-else> ::= 'if' '(' <expression> ')' <statement> { 'else' <statement> }
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseIfElse(SymbolTable* scope) {
    TreeNode* ifblock = new TreeNode(getToken(), TreeNodeType::IF_ELSE, scope); 
    next();
    checkToken(TokenType::OP_PARENTHESES, "Opening parentheses '(' expected");
    next();	
    ifblock->addChild(parseCondition(scope));
    checkToken(TokenType::CL_PARENTHESES, "Closing parentheses ')' expected");
    next();	
    ifblock->addChild(parseStatement(scope));
    if (getNextToken().type == TokenType::ELSE) {
        next(); next();
        ifblock->addChild(parseStatement(scope));
    }
    return ifblock;
}


//---------------------------------------------------------------------------
// <while> :: = 'while' '(' < expression > ')' < statement >
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseWhile(SymbolTable* scope) {
    TreeNode* whileBlock = new TreeNode(getToken(), TreeNodeType::WHILE, scope); 
    next();
    checkToken(TokenType::OP_PARENTHESES, "Opening parentheses '(' expected");
    next(); 
    whileBlock->addChild(parseCondition(scope));
    checkToken(TokenType::CL_PARENTHESES, "Closing parentheses ')' expected");
    next(); 
    whileBlock->addChild(parseStatement(scope));
    return whileBlock;
}


//---------------------------------------------------------------------------
// <assign> ::= <identifier> = <expression> ';'
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseAssignment(SymbolTable* scope) {
    Token identifier = getToken(); 
    if (scope->lookupSymbol(identifier) == NULL) {
        raiseError("Symbol not defined.");
    }
    next();
    checkToken(TokenType::ASSIGN, "Assignment operator '=' expected");
    TreeNode* op = new TreeNode(getToken(), TreeNodeType::ASSIGNMENT, scope); 
    next();
    TreeNode* a = new TreeNode(identifier, TreeNodeType::SYMBOL, scope);
    TreeNode* b = parseCondition(scope);
    op->addChild(a);
    op->addChild(b);
    return op;
}


//---------------------------------------------------------------------------
// <condition> ::= <expression> {( == | != | > | >= | < | <= | && | '||') <expression>}
// todo separate compare and boolean operators by priority
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseCondition(SymbolTable* scope) {
    TreeNode* operand1, * operand2, * op = NULL, * prevOp = NULL;
    operand1 = parseExpression(scope);
    Token token = getToken();
    while (isLogicOp(token.type)) {
        next();
        operand2 = parseExpression(scope);
        op = new TreeNode(token, TreeNodeType::BINARY_OP, scope);
        if (prevOp == NULL) op->addChild(operand1); else op->addChild(prevOp);
        op->addChild(operand2);
        prevOp = op;
        token = getToken();
    }
    return op == NULL ? operand1 : op;

}


//---------------------------------------------------------------------------
// <expression> ::= <term> {(+|-) <term>}
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseExpression(SymbolTable* scope) {
    TreeNode* operand1, * operand2, * op = NULL, * prevOp = NULL;
    operand1 = parseTerm(scope);
    Token token = getToken();
    while (isTokenType(TokenType::PLUS) || isTokenType(TokenType::MINUS)) {
        next();
        operand2 = parseTerm(scope);
        op = new TreeNode(token, TreeNodeType::BINARY_OP, scope);
        if (prevOp == NULL) op->addChild(operand1); else op->addChild(prevOp);
        op->addChild(operand2);
        prevOp = op;
        token = getToken();
    }
    return op == NULL ? operand1 : op;
}


//---------------------------------------------------------------------------
// <term>  ::= <bitwise> {(*|/) <bitwise>}
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseTerm(SymbolTable* scope) {
    TreeNode* operand1, * operand2, * op = NULL, * prevOp = NULL;
    operand1 = parseBitwise(scope);
    Token token = getToken();
    while (isTokenType(TokenType::MULTIPLY) || isTokenType(TokenType::DIVIDE)) {
        next();
        operand2 = parseBitwise(scope);
        op = new TreeNode(token, TreeNodeType::BINARY_OP, scope);
        if (prevOp == NULL) op->addChild(operand1); else op->addChild(prevOp);
        op->addChild(operand2);
        prevOp = op;
        token = getToken(currentToken);
    }
    return op == NULL ? operand1 : op;
}


//---------------------------------------------------------------------------
// <bitwise>  ::= <factor> {( & | '|' | ^ | ~ | << | >> ) <factor>}
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseBitwise(SymbolTable* scope) {
    // todo bitwise operators
    return parseFactor(scope); 
}


//---------------------------------------------------------------------------
// <factor> ::= ({!|-|+} <number>) | <identifer> | <call>
//---------------------------------------------------------------------------
TreeNode* SourceParser::parseFactor(SymbolTable* scope) {

    // TODO add constant to symbol table 

    TreeNode* factor = NULL;
    bool unaryMinus = false;

    if (isTokenType(TokenType::MINUS)) { unaryMinus = true; next(); }
    else
        if (isTokenType(TokenType::PLUS)) { unaryMinus = false; next(); }
        else {
            // TODO add unary Not operator	
        }

    if (isTokenType(TokenType::OP_PARENTHESES)) {
        next();
        factor = parseExpression(scope);
        Token token = getToken();
        if (isTokenType(TokenType::CL_PARENTHESES)) next();
        else raiseError("Closing parentheses expected");
    }
    else if (isTokenType(TokenType::CONST_INTEGER)) {
        factor = new TreeNode(getToken(), TreeNodeType::CONSTANT, scope); next();
    }
    else if (isTokenType(TokenType::IDENTIFIER)) {
        Token nextToken = getNextToken();
        if (nextToken.type == TokenType::OP_PARENTHESES) {
            factor = parseCall(scope); next();
        }
        else {
            factor = new TreeNode(getToken(), TreeNodeType::SYMBOL, scope); 
            if (scope->lookupSymbol(getToken()) == NULL) {
                raiseError("Symbol not defined.");
            }
            next();
        }
    }
    else raiseError("Number or identifier expected");

    if (unaryMinus) {
        Token token = getToken();
        TreeNode* expr = new TreeNode(TKN_MINUS, TreeNodeType::BINARY_OP, scope);
        TreeNode* zero = new TreeNode(TKN_ZERO, TreeNodeType::CONSTANT, scope);
        expr->addChild(zero);
        expr->addChild(factor);
        return expr;
    }

    return factor;

}
