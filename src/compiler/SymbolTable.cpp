/*============================================================================
*
*  Virtual Machine Compiler Symbol Table implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include "compiler/SourceParser.h"

#include <iostream>

using namespace vm;
using namespace std;


SymbolTable::SymbolTable(string name) {
    parent = NULL;
    this->name = name;
   // cout << "Symbol table '" << name << "' created." << endl;
}


SymbolTable::~SymbolTable() {
    SymbolTable* child;
    size_t count = getChildCount();
   // cout << "Symbol table '" << name << "' and " << count << " child deleted." << endl;
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
    entry.localIndex = (int) getNextIndex(type);
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


Symbol* SymbolTable::lookupSymbol(char* name, SymbolType type) {
    Symbol entry;
    size_t count = getSymbolsCount();
    size_t length = strlen(name);
    bool equalName;
    // Search symbol in current scope
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        if (entry.name.size() == length) {
            equalName = strncmp(entry.name.c_str(), name, length) == 0;
            if (equalName && entry.type==type) return &symbols.at(i);
            
        }
    }
    // Search symbol in parent scope
    if (parent != NULL) {
        Symbol* entry = parent->lookupSymbol(name, type);
        if (entry != NULL) return entry;
    }
    return NULL;

}

int SymbolTable::getNextIndex(SymbolType type) {
    Symbol entry;
    size_t count = getSymbolsCount();
    int index = 0;
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        if (entry.type == type) index++;
    }
    return index;
}

void SymbolTable::printSymbols() {
    cout << "-----------------------------------------------------" << endl;
    cout << "Symbol table" << endl;
    cout << "-----------------------------------------------------" << endl;
    printRecursive(0);
}


void SymbolTable::printRecursive(int depth) {
    Symbol entry;
    size_t count = getSymbolsCount();
    for (int i = 0; i < depth; i++) cout << "\t";
    cout << name << ":" << endl;
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        for (int j = 0; j < depth; j++) cout << "\t";
        cout << entry.name << "\t";
        cout << SYMBOL_TYPE_MNEMONIC[(int)entry.type];
        if (entry.type == SymbolType::FUNCTION) {
            cout << " at [" << entry.address << "]";
            cout << " args=" << entry.argCount;
        } else {
            cout << " #" << entry.localIndex;
        }
        
        cout << endl;
    }
    for (int i = 0; i < childs.size(); i++) {
       childs.at(i)->printRecursive(depth + 1);
    }
}