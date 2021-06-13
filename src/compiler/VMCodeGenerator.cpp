/*============================================================================
*
*Virtual Machine Compiler code generator imlementation
*
*(C)Bolat Basheyev 2021
*
============================================================================*/

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
        if (node->getType() == VMNodeType::FUNCTION) {
            VMNode* body = node->getChild(2); // 0 - type, 1 - parameters, 2 - body
            for (int j = 0; j < body->getChildCount(); j++) {
                VMNode* statement = body->getChild(j);
                if (statement->getType() == VMNodeType::ASSIGNMENT) {
                    cout << endl;
                    emitAssignment(statement);
                }
            }
        }

    }
}


void VMCodeGenerator::emitFunction(VMNode* assignment) {

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

