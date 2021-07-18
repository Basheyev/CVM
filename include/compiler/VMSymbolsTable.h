/*============================================================================
*
*  Virtual Machine Compiler code generator header
*
*  (C) Bolat Basheyev 2021
* 
*  TODO implement scope tree search
*  TODO move to parser
*
============================================================================*/
#pragma once

#include "runtime/VMRuntime.h"
#include "compiler/VMLexer.h"

namespace vm {

    enum class SymbolType {
        CONSTANT, FUNCTION, ARGUMENT, VARIABLE
    };

    constexpr char* const  SYMBOL_TYPE_MNEMONIC[] = {
        "CONSTANT", "FUNCTION", "ARGUMENT", "VARIABLE"
    };

    typedef struct {
        char* name;
        WORD length;
        SymbolType type;
        WORD localIndex;
        WORD address;
    } VMSymbolEntry;


    class VMSymbolsTable {
    public:
        VMSymbolsTable();
        ~VMSymbolsTable();
        
        bool addChild(VMSymbolsTable* child);
        bool removeChild(VMSymbolsTable* child);
        VMSymbolsTable* getChildAt(size_t index);
        size_t getChildCount();

        void clearSymbols();
        size_t getSymbolsCount();
        bool addSymbol(Token& token, SymbolType type);
        VMSymbolEntry* lookupSymbol(Token& token);
        VMSymbolEntry* getSymbolAt(WORD index);
        void printSymbols();
    private:
        vector<VMSymbolEntry> symbols;
        vector<VMSymbolsTable*> childs;
        VMSymbolsTable* parent;

        WORD getNextIndex(SymbolType type);
    };

}