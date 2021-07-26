/*============================================================================
*
*  Virtual Machine Compiler code generator header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include "runtime/ExecutableImage.h"
#include "compiler/SourceParser.h"


namespace vm {

    typedef struct  {
        Token token;
        char* error;
    } CodeGeneratorException;

    class CodeGenerator {
    public:
        CodeGenerator();
        ~CodeGenerator();
        void generateCode(ExecutableImage* img, TreeNode* rootNode);

    private:
        SymbolTable symbolsRoot;

        void emitModule(TreeNode* rootNode);
        void emitFunction(TreeNode* assignment);
        void emitBlock(TreeNode* body, SymbolTable* symbols);
        void emitDeclaration(TreeNode* node, SymbolTable* symbols);
        void emitCall(TreeNode* node, SymbolTable* symbols);
        void emitIfElse(TreeNode* node, SymbolTable* symbols);
        void emitWhile(TreeNode* node, SymbolTable* symbols);
        void emitReturn(TreeNode* node, SymbolTable* symbols);
        void emitAssignment(TreeNode* assignment, SymbolTable* symbols);
        void emitExpression(TreeNode* expression, SymbolTable* symbols);
        void emitSymbol(TreeNode* node, SymbolTable* symbols);

        WORD getOpCode(Token& token);

        inline void raiseError(Token& token, char* msg) { throw CodeGeneratorException{ token, msg }; }
    };

}