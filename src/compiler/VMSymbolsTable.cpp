
#include "compiler/VMSymbolsTable.h"

#include <iostream>

using namespace vm;
using namespace std;


VMSymbolsTable::VMSymbolsTable() {
    parent = NULL;
}


VMSymbolsTable::~VMSymbolsTable() {
    VMSymbolsTable* child;
    size_t count = getChildCount();
    for (int i = 0; i < count; i++) {
        child = getChildAt(i);
        delete child;
    }
}



bool VMSymbolsTable::addChild(VMSymbolsTable* child) {
    if (child == NULL) return false;
    childs.push_back(child);
    return true;
}


bool VMSymbolsTable::removeChild(VMSymbolsTable* child) {
  
    // TODO
    
    return false;
}


VMSymbolsTable* VMSymbolsTable::getChildAt(size_t index) {
    return childs.at(index);
}

size_t VMSymbolsTable::getChildCount() {
    return childs.size();
}



void VMSymbolsTable::clearSymbols() {
    symbols.clear();
}


size_t VMSymbolsTable::getSymbolsCount() {
    return symbols.size();
}

bool VMSymbolsTable::addSymbol(Token& token, SymbolType type) {
    if (lookupSymbol(token) != NULL) return false;
    VMSymbolEntry entry = {token.text, token.length, type, getNextIndex(type), NULL};
    symbols.push_back(entry);
    return true;
}


VMSymbolEntry* VMSymbolsTable::getSymbolAt(WORD index) {
    return &symbols.at(index);
}


VMSymbolEntry* VMSymbolsTable::lookupSymbol(Token& token) {
    VMSymbolEntry entry;
    size_t count = getSymbolsCount();
    size_t length;
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        if (entry.length == token.length) {
            length = token.length;
            if (strncmp(entry.name, token.text, length)==0) return &symbols.at(i);
        }
    }
    if (parent != NULL) {
        VMSymbolEntry* entry = parent->lookupSymbol(token);
        if (entry != NULL) return entry;
    }
    return NULL;
}


WORD VMSymbolsTable::getNextIndex(SymbolType type) {
    VMSymbolEntry entry;
    size_t count = getSymbolsCount();
    size_t index = 0;
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        if (entry.type == type) index++;
    }
    return index;
}

void VMSymbolsTable::printSymbols() {
    VMSymbolEntry entry;
    size_t count = getSymbolsCount();
    cout << "------------------ SYMBOLS TABLE ------------------" << endl;
    for (int i = 0; i < count; i++) {
        entry = symbols.at(i);
        cout << i << "\t";
        cout.write(entry.name, entry.length);
        cout << "\t";
        cout << SYMBOL_TYPE_MNEMONIC[(int)entry.type];
        cout << "\t";
        cout << "index=" << entry.localIndex;
        cout << endl;
    }
    for (int i = 0; i < childs.size(); i++) {
        childs.at(i)->printSymbols();
    }
}