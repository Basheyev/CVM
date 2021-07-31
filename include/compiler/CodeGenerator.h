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
        void emitModule(TreeNode* rootNode);
        void emitFunction(TreeNode* assignment);
        void emitBlock(TreeNode* body);
        void emitDeclaration(TreeNode* node);
        void emitCall(TreeNode* node);
        void emitIfElse(TreeNode* node);
        void emitWhile(TreeNode* node);
        void emitReturn(TreeNode* node);
        void emitAssignment(TreeNode* assignment);
        void emitExpression(TreeNode* expression);
        void emitSymbol(TreeNode* node);
        WORD getOpCode(Token& token);

    private:
        inline void raiseError(Token& token, char* msg) { throw CodeGeneratorException{ token, msg }; }
    };

}