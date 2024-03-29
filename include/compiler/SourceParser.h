/*============================================================================
*
*  Virtual Machine Compiler Source Code Parser header
*
*  Basic C like language grammar:
* 
*  <module>      ::= {<function>}*
*  <type>        ::= 'int'
*  <function>    ::= <type> <identifier> '(' <argument> {, <argument>}* ')' <block>
*  <argument>    ::= <type> <identifier>
*  <statement>   ::= <block> | <declaration> | <assign> | <if-else> | <while> | <jump> | <call>
*  <declaration> ::= <type> <identifier> {','<identifier>}* ';'
*  <block>       ::= '{' {<statement>}* '}'
*  <call>        ::= <identifier> '(' {<expression>} {, expression}* ')'
*  <if-else>     ::= 'if' '(' <condition> ')' <statement> { 'else' <statement> }
*  <while>       ::= 'while' '(' <condition> ')' <statement>
*  <jump>        ::= 'return' <expression> ';' | 'break' ';'
*  <assign>      ::= <identifier> = <expression> ';'
*  <logical>     ::= <comparison> {( && | '||') <comparison>}
*  <comparison>  ::= <expression> {( == | != | > | >= | < | <= ) <expression>}
*  <expression>  ::= <term> {(+|-) <term>}
*  <term>        ::= <bitwise> {(*|/) <bitwise>}
*  <bitwise>     ::= <factor> {( & | '|' | ^ | << | >> ) <factor>}
*  <factor>      ::= ({~|!|-|+} <number>) | <identifer> | <call>
* 
* 
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include <string>
#include <vector>
#include <cstring>

#include "runtime/VirtualMachine.h"

using namespace std;

namespace vm {
    
    constexpr char* BLANKS = "\x20\n\r\t";
    constexpr char* DELIMETERS = ",;{}[]()=><+-*/&|~^!";

    //------------------------------------------------------------------------
    // Tokens
    //------------------------------------------------------------------------
    enum class TokenType {
        EMPTY = 0, NONE, UNKNOWN, IDENTIFIER, CONST_INTEGER, CONST_STRING, COMMA, EOS,
        OP_BRACES, CL_BRACES, OP_BRACKETS, CL_BRACKETS, OP_PARENTHESES, CL_PARENTHESES,
        INT, IF, ELSE, WHILE, RETURN, BREAK, ASSIGN, PLUS, MINUS, MULTIPLY, DIVIDE, NOT, AND, OR, XOR, SHL, SHR,
        EQUAL, NOT_EQUAL, GREATER, GR_EQUAL, LESS, LS_EQUAL, LOGIC_AND, LOGIC_OR, LOGIC_NOT
    };

    constexpr char* const TOKEN_TYPE_MNEMONIC[] = {
        "", "", "", "", "", "", ",", ";",
        "{", "}", "[", "]", "(", ")",
        "int", "if", "else", "while", "return", "break",
        "=", "+", "-", "*", "/", "~", "&", "|", "^", "<<", ">>",
        "==", "!=", ">", ">=", "<", "<=", "&&", "||", "!"
    };

    constexpr int TOKEN_TYPE_COUNT = sizeof(TOKEN_TYPE_MNEMONIC) / sizeof(char*);


    class Token {
    public:
        TokenType type;
        char* text;
        int length;
        int row, col;
    };

    constexpr Token EMPTY_TOKEN = { TokenType::EMPTY, "", 0, 0, 0 };
    constexpr Token TKN_ARGUMENTS = { TokenType::NONE, "ARGUMENTS", 9, 0, 0 };
    constexpr Token TKN_BLOCK = { TokenType::NONE, "BLOCK", 5, 0, 0 };
    constexpr Token TKN_ZERO = { TokenType::CONST_INTEGER, "0", 1, 0, 0 };
    constexpr Token TKN_MINUS = { TokenType::MINUS, "-", 1, 0, 0 };
    constexpr Token TKN_BITWISE_NOT = { TokenType::NOT, "~", 1, 0, 0 };
    constexpr Token TKN_LOGICAL_NOT = { TokenType::LOGIC_NOT, "!", 1, 0, 0 };

    //------------------------------------------------------------------------
    // Symbol table
    //------------------------------------------------------------------------
    enum class SymbolType {
        UNKNOWN, CONSTANT, FUNCTION, ARGUMENT, VARIABLE
    };

    constexpr char* const  SYMBOL_TYPE_MNEMONIC[] = {
        "UNKNOWN", "CONSTANT", "FUNCTION", "ARGUMENT", "VARIABLE"
    };

    class Symbol {
    public:
        string name = "";
        SymbolType type = SymbolType::UNKNOWN;
        WORD localIndex = -1;
        WORD address = -1;
        WORD argCount = 0;
        
    };

    class SymbolTable {
    public:

        SymbolTable(string name = "GLOBAL");
        ~SymbolTable();

        bool addChild(SymbolTable* child);
        void removeChild(SymbolTable* child);
        SymbolTable* getChildAt(size_t index);
        size_t getChildCount();

        inline const char* getName() { return name.c_str(); };
        void clearSymbols();
        size_t getSymbolsCount();
        bool addSymbol(Token& token, SymbolType type);
        Symbol* lookupSymbol(Token& token);
        Symbol* lookupSymbol(char* name, SymbolType type);
        Symbol* getSymbolAt(size_t index);
        void printSymbols();

    private:
        string name;
        vector<Symbol> symbols;
        vector<SymbolTable*> childs;
        SymbolTable* parent;
        int getNextIndex(SymbolType type);
        void printRecursive(int depth);
    };

    //------------------------------------------------------------------------
    // Abstract Syntax Tree Node
    //------------------------------------------------------------------------

    enum class TreeNodeType {
        UNKNOWN = 0, MODULE, CONSTANT, TYPE, SYMBOL, UNARY_OP, BINARY_OP, CALL,
        FUNCTION, BLOCK, ASSIGNMENT, IF_ELSE, WHILE, RETURN, BREAK
    };

    constexpr char* const TREE_NODE_TYPE_MNEMONIC[] = {
        "UNKNOWN", "MODULE", "CONSTANT", "TYPE", "SYMBOL", "UNARY_OP", "BINARY_OP", "CALL",
        "FUNCTION", "BLOCK", "ASSIGNMENT", "IF_ELSE", "WHILE", "RETURN", "BREAK"
    };

    class TreeNode {
    public:
        TreeNode(Token token, TreeNodeType type, SymbolTable* scope);
        ~TreeNode();
        TreeNode* addChild(TreeNode* node);
        bool removeChild(TreeNode* node);
        void removeAll();
        TreeNodeType getType();
        Token &getToken();
        TreeNode* getParent();
        TreeNode* getChild(size_t index);
        size_t getChildCount();
        size_t getDepth();
        void print();
        inline void setSymbolTable(SymbolTable* scope) { symbols = scope; }
        inline SymbolTable* getSymbolTable() { return symbols; }
    private:
        Token token;
        SymbolTable* symbols = NULL;
        vector<TreeNode*> childs;
        TreeNode* parent;
        TreeNodeType type;
        void print(int tab);
    };

    //------------------------------------------------------------------------
    // Parser exception
    //------------------------------------------------------------------------
    class ParserException {
    public:
        Token& token;
        const char* msg;
    };

    //------------------------------------------------------------------------
    // Syntax Parser (Abstract Syntax Tree Builder)
    //------------------------------------------------------------------------
    class SourceParser {
    public:
        SourceParser(const char* sourceCode);
        ~SourceParser();
        inline size_t getTokenCount() { return tokens.size(); }
        Token& getToken(size_t index) { return tokens[index]; }      // FIXME: not sure
        SymbolTable& getSymbolTable() { return rootSymbolTable; }
        TreeNode* getSyntaxTree() { return root; }
    private:
        vector<Token> tokens;
        TreeNode* root = NULL;
        SymbolTable rootSymbolTable;
        size_t currentToken = 0;
        int blockCounter = 0;

        void parseToTokens(const char* sourceCode);
        bool isBlank(char value) { return strchr(BLANKS, value) != NULL; };
        bool isDelimeter(char value) { return strchr(DELIMETERS, value) != NULL; };
        bool pushToken(char* text, int length, int row, int col);
        TokenType getTokenType(char* text, int length);
        TokenType validateNumber(char* text, int length);
        TokenType validateIdentifier(char* text, int length);
        TokenType validateString(char* text, int length);

        void buildSyntaxTree();
        TreeNode* parseModule(SymbolTable* scope);
        TreeNode* parseDeclaration(SymbolTable* scope);
        TreeNode* parseFunction(SymbolTable* scope);
        TreeNode* parseArgument(SymbolTable* scope);
        TreeNode* parseBlock(SymbolTable* scope, bool isFunction, bool whileBlock);
        TreeNode* parseStatement(SymbolTable* scope, bool whileBlock);
        TreeNode* parseCall(SymbolTable* scope);
        TreeNode* parseIfElse(SymbolTable* scope, bool whileBlock);
        TreeNode* parseWhile(SymbolTable* scope);
        TreeNode* parseAssignment(SymbolTable* scope);
        TreeNode* parseLogical(SymbolTable* scope);
        TreeNode* parseComparison(SymbolTable* scope);
        TreeNode* parseExpression(SymbolTable* scope);
        TreeNode* parseTerm(SymbolTable* scope);
        TreeNode* parseBitwise(SymbolTable* scope);
        TreeNode* parseFactor(SymbolTable* scope);

        inline bool next() { currentToken++; return currentToken < getTokenCount(); }
        inline Token& getToken() { return getToken(currentToken); }
        inline Token& getNextToken() { return getToken(currentToken + 1); }
        inline bool isTokenType(TokenType type) { return getToken().type == type; }

        inline bool isComparison(TokenType type) { return type >= TokenType::EQUAL && type <= TokenType::LS_EQUAL; }
        inline bool isLogical(TokenType type) { return type >= TokenType::LOGIC_AND && type <= TokenType::LOGIC_OR; }
        inline bool isBitwise(TokenType type) { return type >= TokenType::AND && type <= TokenType::SHR; }
        inline bool isDataType(TokenType type) { return type == TokenType::INT; }
        inline void checkToken(TokenType type, const char* msg) { if (!isTokenType(type)) raiseError(msg); }
        inline void raiseError(Token& tkn, const char* msg) { throw ParserException{ tkn, msg }; }
        inline void raiseError(const char* msg) { throw ParserException{ getToken(), msg }; }
    };

};
