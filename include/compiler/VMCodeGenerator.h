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


namespace vm {


    class VMSymbolTable {
        char* name;
        int type;
        WORD address;
    };


    class VMCodeGenerator {
    public:
        VMCodeGenerator();
        ~VMCodeGenerator();
        void generateCode(VMImage* img, VMNode* rootNode);

    private:

        void emitFunction(VMNode* assignment);
        void emitIfElse(VMNode* assignment);
        void emitWhile(VMNode* assignment);
        void emitAssignment(VMNode* assignment);
        void emitExpression(VMNode* expression);


        WORD getOpCode(Token& token);

    };

}