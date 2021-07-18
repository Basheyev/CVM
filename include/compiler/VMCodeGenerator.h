/*============================================================================
*
*  Virtual Machine Compiler code generator header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include "image/VMImage.h"
#include "compiler/VMNode.h"
#include "compiler/VMParser.h"
#include "compiler/VMSymbolsTable.h"


namespace vm {

    typedef struct  {
        Token token;
        char* error;
    } VMCodeGeneratorException;

    class VMCodeGenerator {
    public:
        VMCodeGenerator();
        ~VMCodeGenerator();
        void generateCode(VMImage* img, VMNode* rootNode);

    private:
        VMSymbolsTable symbolsRoot;

        void emitModule(VMNode* rootNode);
        void emitFunction(VMNode* assignment);
        void emitBlock(VMNode* body, VMSymbolsTable* symbols);
        void emitDeclaration(VMNode* node, VMSymbolsTable* symbols);
        void emitCall(VMNode* node, VMSymbolsTable* symbols);
        void emitIfElse(VMNode* node, VMSymbolsTable* symbols);
        void emitWhile(VMNode* node, VMSymbolsTable* symbols);
        void emitReturn(VMNode* node, VMSymbolsTable* symbols);
        void emitAssignment(VMNode* assignment, VMSymbolsTable* symbols);
        void emitExpression(VMNode* expression, VMSymbolsTable* symbols);
        void emitSymbol(VMNode* node, VMSymbolsTable* symbols);

        WORD getOpCode(Token& token);

        inline void raiseError(Token& token, char* msg) { throw VMCodeGeneratorException{ token, msg }; }
    };

}