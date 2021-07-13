/*============================================================================
*
*  Virtual Machine Compiler code generator header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include "runtime/VMRuntime.h"
#include "compiler/VMLexer.h"

namespace vm {

    typedef enum {
        FUNCTION, ARGUMENT, VARIABLE
    } SymbolType;

    class VMSymbolsEntry {
        char* name;
        SymbolType type;
        WORD address;
    };


    class VMSymbols {


        bool addEntry(VMSymbolsEntry& entry);
        VMSymbolsEntry& getEntry();



    };

}