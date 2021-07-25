/*============================================================================
*
*  Virtual Machine Compiler Source Code Parser header
*
*  Basic C like language grammar:
* 
*  <module>          ::= {<declaration>|<function>}*
*  <type>            ::= 'int'
*  <declaration>     ::= <type> <identifier> {','<identifier>}* ';' 
*  <function>        ::= <type> <identifier> '(' <argument> {, <argument>}* ')' <block>
*  <argument>        ::= <type> <identifier>
*  <statement>       ::= <block> | <declration> | <assign> | <if-else> | <while> | <jump> | <call>)
*  <block>           ::= '{' {<statement>}* '}'
*  <call>            ::= <identifier> '(' {<expression>} {, expression}* ')'
*  <if-else>         ::= 'if' '(' <expression> ')' <statement> { 'else' <statement> }
*  <while>           ::= 'while' '(' <expression> ')' <statement>
*  <jump>            ::= 'return' <expression> ';'
*  <assign>          ::= <identifier> = <expression> ';'
*  <condition>       ::= <expression> {( == | != | > | >= | < | <= | && | '||' | !) <expression>}
*  <expression>      ::= <term> {(+|-) <term>}
*  <term>            ::= <bitwise> {(*|/) <bitwise>}
*  <bitwise>         ::= <factor> {( & | '|' | ^ | ~ | << | >> ) <factor>}
*  <factor>          ::= ({-|+} <number>) | <identifer> | <call>
* 
* 
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include <string>
#include <vector>

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
        INT, IF, ELSE, WHILE, RETURN, ASSIGN, PLUS, MINUS, MULTIPLY, DIVIDE, AND, OR, XOR, NOT, SHL, SHR,
        EQUAL, NOT_EQUAL, GREATER, GR_EQUAL, LESS, LS_EQUAL, LOGIC_AND, LOGIC_OR, LOGIC_NOT
    };

    constexpr char* const TOKEN_TYPE_MNEMONIC[] = {
        "", "", "", "", "", "", ",", ";",
        "{", "}", "[", "]", "(", ")",
        "int", "if", "else", "while", "return",
        "=", "+", "-", "*", "/", "&", "|", "^", "~", "<<", ">>",
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

    //------------------------------------------------------------------------
    // Symbol Table
    //------------------------------------------------------------------------
    enum class SymbolType {
        CONSTANT, VARIABLE, FUNCTION
    };

    class Symbol {
    public:
        Token token;
        SymbolType type;
    };

    class SymbolTable {
    public:
        vector<Symbol> symbols;
        vector<SymbolTable*> childs;
        SymbolTable* parent;
    };

    //------------------------------------------------------------------------
    // Abstract Syntax Tree Node
    //------------------------------------------------------------------------
    enum class TreeNodeType {
        UNKNOWN = 0, MODULE, CONSTANT, TYPE, SYMBOL, BINARY_OP, CALL,
        FUNCTION, BLOCK, IF_ELSE, WHILE, RETURN
    };

    constexpr char* const TREE_NODE_TYPE_MNEMONIC[] = {
        "UNKNOWN", "MODULE", "CONSTANT", "TYPE", "SYMBOL", "BINARY_OP", "CALL",
        "FUNCTION", "BLOCK", "IF_ELSE", "WHILE", "RETURN"
    };

    class TreeNode {
    public:
        TreeNode(Token token, TreeNodeType type);
        ~TreeNode();
        TreeNode* addChild(TreeNode* node);
        bool removeChild(TreeNode* node);
        void removeAll();
        TreeNodeType getType();
        Token getToken();
        TreeNode* getParent();
        TreeNode* getChild(size_t index);
        size_t getChildCount();
        size_t getDepth();
        void print();
    private:
        Token token;
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
        Token& getToken(size_t index) { return tokens[index]; }
        SymbolTable& getSymbolTable() { return symbols; }
        TreeNode* getSyntaxTree() { return root; }
    private:
        vector<Token> tokens;
        TreeNode* root = NULL;
        SymbolTable symbols;
        size_t currentToken = 0;

        void parseToTokens(const char* sourceCode);
        bool isBlank(char value) { return strchr(BLANKS, value) != NULL; };
        bool isDelimeter(char value) { return strchr(DELIMETERS, value) != NULL; };
        bool pushToken(char* text, int length, int row, int col);
        TokenType getTokenType(char* text, int length);
        TokenType identifyNumber(char* text, int length);
        TokenType identifyKeyword(char* text, int length);
        TokenType identifyString(char* text, int length);

        void buildSyntaxTree();
        TreeNode* parseModule();
        TreeNode* parseDeclaration();
        TreeNode* parseFunction();
        TreeNode* parseArguments();
        TreeNode* parseCall();
        TreeNode* parseBlock();
        TreeNode* parseStatement();
        TreeNode* parseIfElse();
        TreeNode* parseWhile();
        TreeNode* parseAssignment();
        TreeNode* parseCondition();
        TreeNode* parseExpression();
        TreeNode* parseTerm();
        TreeNode* parseBitwise();
        TreeNode* parseFactor();

        inline bool next() { currentToken++; return currentToken < getTokenCount(); }
        inline Token getToken() { return getToken(currentToken); }
        inline Token getNextToken() { return getToken(currentToken + 1); }
        inline bool isTokenType(TokenType type) { return getToken().type == type; }
        inline bool isLogicOp(TokenType type) { return type >= TokenType::EQUAL && type <= TokenType::LOGIC_NOT; }
        inline bool isDataType(TokenType type) { return type == TokenType::INT; }
        inline void checkToken(TokenType type, const char* msg) { if (!isTokenType(type)) raiseError(msg); }
        inline void raiseError(Token& tkn, const char* msg) { throw ParserException{ tkn, msg }; }
        inline void raiseError(const char* msg) { throw ParserException{ getToken(), msg }; }
    };

};