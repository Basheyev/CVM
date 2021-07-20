/*============================================================================
*
*  Virtual Machine Compiler code generator imlementation
*
*  (C)Bolat Basheyev 2021
*
============================================================================*/

// Questions
// 1. How to replace symbols by addresses?
// 2. How to layout code in image?
//    a) Maybe generate vector of opcode of body for each function (with symbols)
//    b) Collect all variables and constants in symbol table with addresses
//    c) Replace symbols with addresses
// 3. I have 3 enumeration sets: OP_CODES, TokenType, NodeType (which one to use add Node to get better code generation?)


#include "compiler/VMCodeGenerator.h"

#include <iostream>


using namespace vm;
using namespace std;

int blockCounter = 0;

VMCodeGenerator::VMCodeGenerator() {

}

VMCodeGenerator::~VMCodeGenerator() {


}


void VMCodeGenerator::generateCode(VMImage* img, VMNode* rootNode) {
    try {
        rootNode->print();
        emitModule(rootNode);
    }
    catch (VMCodeGeneratorException& e) {
        cout << "CODE GENERATION ERROR: " << endl;
        cout.write(e.token.text, e.token.length);
        cout << endl << e.error << endl;
    }
    symbolsRoot.printSymbols();
}


void VMCodeGenerator::emitModule(VMNode* rootNode) {
    for (int i = 0; i < rootNode->getChildCount(); i++) {
        VMNode* node = rootNode->getChild(i);
        if (node->getType() == VMNodeType::FUNCTION) {
            Token tkn = node->getToken();
            cout << endl;
            cout.write(tkn.text, tkn.length);
            cout << ":" << endl;
            emitFunction(node);
        }
        else if (node->getType() == VMNodeType::DATA_TYPE) {
            emitDeclaration(node, &symbolsRoot);
        }
    }
}


// Node Childs: #0 - type, #1 - arguments, #2 - function body
void VMCodeGenerator::emitFunction(VMNode* node) {
    
    // add function to symbol table
    Token funcToken = node->getToken();
    if (!symbolsRoot.addSymbol(funcToken, SymbolType::FUNCTION)) raiseError(funcToken, "Function already defined");
   
    // create function symbol table
    VMSymbolsTable* symbols = new VMSymbolsTable();
    symbolsRoot.addChild(symbols);

    // add arguments to symbol table
    VMNode* arguments = node->getChild(1);
    Token token;
    for (int i = 0; i < arguments->getChildCount(); i++) {
        token = arguments->getChild(i)->getChild(0)->getToken();
        if (!symbols->addSymbol(token, SymbolType::ARGUMENT)) raiseError(token, "Argument already defined");
    }

    VMNode* body = node->getChild(2);
    emitBlock(body, symbols);
}


void VMCodeGenerator::emitBlock(VMNode* body, VMSymbolsTable* symbols) {
    // emit function body
    for (int j = 0; j < body->getChildCount(); j++) {
        VMNode* statement = body->getChild(j);
        if (statement->getType() == VMNodeType::DATA_TYPE) emitDeclaration(statement, symbols);
        else if (statement->getType() == VMNodeType::ASSIGNMENT) emitAssignment(statement, symbols);
        else if (statement->getType() == VMNodeType::RETURN) emitReturn(statement, symbols);
        else if (statement->getType() == VMNodeType::IF_STATEMENT) emitIfElse(statement, symbols);
        else if (statement->getType() == VMNodeType::BLOCK) {
            cout << "block" << blockCounter++ << ":" << endl;
            emitBlock(statement, symbols);
        }
        else {
            // todo other statements

        }
    }
}


void VMCodeGenerator::emitDeclaration(VMNode* node, VMSymbolsTable* symbols) {
    Token token;
    for (int i = 0; i < node->getChildCount(); i++) {
        token = node->getChild(i)->getToken();
        if (!symbols->addSymbol(token, SymbolType::VARIABLE)) raiseError(token, "Variable already defined");
        cout << "iconst 0      // int ";
        cout.write(token.text, token.length);
        cout << ";" << endl;
    }
}

void VMCodeGenerator::emitCall(VMNode* node, VMSymbolsTable* symbols) {
    for (int i = 0; i < node->getChildCount(); i++) {
        emitExpression(node->getChild(i), symbols);
    }
    cout << "call ";
    Token tkn = node->getToken();
    cout.write(tkn.text, tkn.length);
    cout << endl;
}


void VMCodeGenerator::emitIfElse(VMNode* node, VMSymbolsTable* symbols) {
    VMNode* condition = node->getChild(0);
    VMNode* thenBlock = node->getChild(1);
    VMNode* elseBlock = node->getChild(2);
    emitExpression(condition, symbols);
    cout << "cmpje  [ addr  ]" << endl;
    emitBlock(thenBlock, symbols);
    emitBlock(elseBlock, symbols);
    // address + length1;
}


void VMCodeGenerator::emitWhile(VMNode* node, VMSymbolsTable* symbols) {

}

void VMCodeGenerator::emitReturn(VMNode* node, VMSymbolsTable* symbols) {
    emitExpression(node->getChild(0), symbols);
    cout << "ret  ";
    cout << endl;
}


void VMCodeGenerator::emitAssignment(VMNode* assignment, VMSymbolsTable* symbols) {
    Token asgn = assignment->getChild(0)->getToken();
    emitExpression(assignment->getChild(1), symbols);
    VMSymbolEntry* entry = symbols->lookupSymbol(asgn);
    if (entry != NULL) {
        cout << "istore ";
        cout << entry->localIndex;
        cout << endl;
    }
    else {

    }
}


void VMCodeGenerator::emitExpression(VMNode* node, VMSymbolsTable* symbols) {
    size_t childCount = node->getChildCount();
    if (childCount == 0) {
        emitSymbol(node, symbols);
    } else if (node->getType() == VMNodeType::BINARY_OPERATION && childCount == 2) {
        emitExpression(node->getChild(0), symbols);
        emitExpression(node->getChild(1), symbols);
        getOpCode(node->getToken());
    } else if (node->getType() == VMNodeType::CALL) {
        emitCall(node, symbols);
    } else {
        cout << "Error unknown Node" << endl;
    }
}


void VMCodeGenerator::emitSymbol(VMNode* node, VMSymbolsTable* symbols) {
    Token token = node->getToken();
    if (node->getType() == VMNodeType::SYMBOL) {
        VMSymbolEntry* entry = symbols->lookupSymbol(token);
        if (entry != NULL) {
            if (entry->type == SymbolType::ARGUMENT) {
                cout << "iarg  ";
                cout << entry->localIndex;
            }
            if (entry->type == SymbolType::VARIABLE) {
                cout << "iload ";
                cout << entry->localIndex;
            }
        }
    }
    else {
        cout << "iconst ";
        cout.write(token.text, token.length);
    }
    cout << endl;
}


WORD VMCodeGenerator::getOpCode(Token& token) {
    switch (token.type) {
    case TokenType::PLUS: cout << "iadd" << endl; break;
    case TokenType::MINUS: cout << "isub" << endl; break;
    case TokenType::MULTIPLY: cout << "imul" << endl; break;
    case TokenType::DIVIDE: cout << "idiv" << endl; break;
    default:
        cout << "UNKNOWN BINARY OPERATION: ";
        cout.write(token.text, token.length);
        cout << endl;
        break;
    }
    return 0;
}

