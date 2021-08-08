/*============================================================================
*
*  Virtual Machine Compiler code generator header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include "runtime/VirtualMachine.h"
#include "compiler/SourceParser.h"


namespace vm {

    typedef struct  {
        char* error;
    } CodeGeneratorException;

    class CodeGenerator {
    public:
        CodeGenerator();
        ~CodeGenerator();
        void generateCode(ExecutableImage* img, TreeNode* rootNode);
        void emitModule(ExecutableImage* img, TreeNode* rootNode);
        void emitFunction(ExecutableImage* img, TreeNode* node);
        void emitStatement(ExecutableImage* img, TreeNode* body);
        void emitBlock(ExecutableImage* img, TreeNode* body);
        void emitDeclaration(ExecutableImage* img, TreeNode* node);
        void emitCall(ExecutableImage* img, TreeNode* node);
        void emitIfElse(ExecutableImage* img, TreeNode* node);
        void emitWhile(ExecutableImage* img, TreeNode* node);
        void emitReturn(ExecutableImage* img, TreeNode* node);
        void emitBreak(ExecutableImage* img, TreeNode* node);
        void emitAssignment(ExecutableImage* img, TreeNode* assignment);
        void emitExpression(ExecutableImage* img, TreeNode* expression);
        void emitSymbol(ExecutableImage* img, TreeNode* node);
        WORD emitOpcode(ExecutableImage* img, Token& token);

    private:
        inline void raiseError(char* msg) { throw CodeGeneratorException{msg }; }
    };

}