/*============================================================================
*
*Virtual Machine Compiler code generator imlementation
*
*(C)Bolat Basheyev 2021
*
============================================================================*/

// Questions
// 1. How to store and access local variables & parameters (recursion)?
//    a) How to store local variables?
//    b) How to pass parameters?
//    c) Do i need FP special VM opcodes to access local variables?
//    d) Is Frame theme has relation to my problem?
// 2. How to replace symbols by addresses?
// 3. How to layout code in image?
//    a) Maybe generate vector of opcode of body for each function (with symbols)
//    b) Collect all variables and constants in symbol table with addresses
//    c) Replace symbols with addresses
// 4. I have 3 enumeration sets: OP_CODES, TokenType, NodeType (which one to use ad Node to get better code generation?)


#include "compiler/VMCodeGenerator.h"

#include <iostream>


using namespace vm;
using namespace std;

VMCodeGenerator::VMCodeGenerator() {

}

VMCodeGenerator::~VMCodeGenerator() {


}


void VMCodeGenerator::generateCode(VMImage* img, VMNode* rootNode) {
    rootNode->print();
    cout << endl;
    for (int i = 0; i < rootNode->getChildCount(); i++) {
        VMNode* node = rootNode->getChild(i);
        if (node->getType() == VMNodeType::FUNCTION) emitFunction(node);
    }
}


void VMCodeGenerator::emitFunction(VMNode* node) {
    // Childs: #0 - type, #1 - parameters, #2 - function body
    VMNode* body = node->getChild(2); 
    for (int j = 0; j < body->getChildCount(); j++) {
        VMNode* statement = body->getChild(j);
        if (statement->getType() == VMNodeType::ASSIGNMENT) {
            cout << endl;
            emitAssignment(statement);
        }
    }
}


void VMCodeGenerator::emitIfElse(VMNode* assignment) {

}


void VMCodeGenerator::emitWhile(VMNode* assignment) {

}


void VMCodeGenerator::emitAssignment(VMNode* assignment) {
    Token asgn = assignment->getChild(0)->getToken();
    emitExpression(assignment->getChild(1));
    cout << "ipop ";
    cout.write(asgn.text, asgn.length);
    cout << endl;
}


void VMCodeGenerator::emitExpression(VMNode* node) {
    int childCount = node->getChildCount();
    if (childCount == 0) {
        Token token = node->getToken();
        if (node->getType() == VMNodeType::SYMBOL) cout << "ipush "; else cout << "iconst ";
        cout.write(token.text, token.length);
        cout << endl;
    } else if (node->getType()== VMNodeType::BINARY_OPERATION && childCount == 2) {
        emitExpression(node->getChild(0));
        emitExpression(node->getChild(1));
        getOpCode(node->getToken());
    } else {
        cout << "Error" << endl;
    }
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

