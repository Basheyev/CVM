/*============================================================================
*
*  Virtual Machine Compiler Symbol Table implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "compiler/SourceParser.h"

#include <iostream>

using namespace vm;
using namespace std;


SymbolTable::SymbolTable(string name) {
    parent = NULL;
    this->name = name;
}


SymbolTable::~SymbolTable() {
    SymbolTable* child;
    size_t count = getChildCount();
    for (int i = 0; i < count; i++) {
        child = getChildAt(i);
        delete child;
    }
}



bool SymbolTable::addChild(SymbolTable* child) {
    if (child == NULL) return false;
    child->parent = this;
    childs.push_back(child);
    return true;
}


void SymbolTable::removeChild(SymbolTable* child) {
    for (auto entry = begin(childs); entry != end(childs); ++entry) {
        if (*entry == child) {
            childs.erase(entry);
            delete child;
            return;
        }
    }
}


SymbolTable* SymbolTable::getChildAt(size_t index) {
    return childs.at(index);
}

size_t SymbolTable::getChildCount() {
    return childs.size();
}



void SymbolTable::clearSymbols() {
    symbols.clear();
}


size_t SymbolTable::getSymbolsCount() {
    return symbols.size();
}

bool SymbolTable::addSymbol(Token& token, SymbolType type) {
    if (lookupSymbol(token) != NULL) return false;
    Symbol entry;
    entry.name.append(token.text, token.length);
    entry.type = type;
    entry.localIndex = getNextIndex(type);
    entry.address = NULL;
    symbols.push_back(entry);
    return true;
}


Symbol* SymbolTable::getSymbolAt(size_t index) {
    return &symbols.at(index);
}


Symbol* SymbolTable::lookupSymbol(Token& token) {
    Symbol entry;
    size_t count = getSymbolsCount();
    size_t length;
    // Search symbol in current scope
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        if (entry.name.size() == token.length) {
            length = token.length;
            if (strncmp(entry.name.c_str(), token.text, length)==0) return &symbols.at(i);
        }
    }
    // Search symbol in parent scope
    if (parent != NULL) {
        Symbol* entry = parent->lookupSymbol(token);
        if (entry != NULL) return entry;
    }
    return NULL;
}


size_t SymbolTable::getNextIndex(SymbolType type) {
    Symbol entry;
    size_t count = getSymbolsCount();
    size_t index = 0;
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        if (entry.type == type) index++;
    }
    return index;
}


void SymbolTable::printSymbols() {
    Symbol entry;
    size_t count = getSymbolsCount();
    cout << "------------------ " << name << " ------------------" << endl;
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        cout << i << "\t" << entry.name << "\t";
        cout << SYMBOL_TYPE_MNEMONIC[(int)entry.type];
        cout << "\t";
        cout << "index=" << entry.localIndex;
        cout << endl;
    }
    for (int i = 0; i < childs.size(); i++) {
        childs.at(i)->printSymbols();
    }
}