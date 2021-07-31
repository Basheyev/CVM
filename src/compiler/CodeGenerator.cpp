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


#include "compiler/CodeGenerator.h"

#include <iostream>


using namespace vm;
using namespace std;

int blockCounter = 0;

CodeGenerator::CodeGenerator() {

}

CodeGenerator::~CodeGenerator() {


}


void CodeGenerator::generateCode(ExecutableImage* img, TreeNode* rootNode) {
    try {
        //rootNode->print();
        emitModule(rootNode);
    }
    catch (CodeGeneratorException& e) {
        cout << "CODE GENERATION ERROR: " << endl;
        cout.write(e.token.text, e.token.length);
        cout << endl << e.error << endl;
    }
}


void CodeGenerator::emitModule(TreeNode* rootNode) {
    for (int i = 0; i < rootNode->getChildCount(); i++) {
        TreeNode* node = rootNode->getChild(i);
        if (node->getType() == TreeNodeType::FUNCTION) {
            Token tkn = node->getToken();
            cout << endl;
            cout.write(tkn.text, tkn.length);
            cout << ":" << endl;
            emitFunction(node);
        }
        else if (node->getType() == TreeNodeType::TYPE) {
            emitDeclaration(node);
        }
    }
}


// Node Childs: #0 - type, #1 - arguments, #2 - function body
void CodeGenerator::emitFunction(TreeNode* node) {
    Token funcToken = node->getToken();
    TreeNode* body = node->getChild(2);
    emitBlock(body);
}


void CodeGenerator::emitBlock(TreeNode* body) {
    // emit function body
    for (int j = 0; j < body->getChildCount(); j++) {
        TreeNode* statement = body->getChild(j);
        if (statement->getType() == TreeNodeType::TYPE) emitDeclaration(statement);
        else if (statement->getType() == TreeNodeType::ASSIGNMENT) emitAssignment(statement);
        else if (statement->getType() == TreeNodeType::RETURN) emitReturn(statement);
        else if (statement->getType() == TreeNodeType::IF_ELSE) emitIfElse(statement);
        else if (statement->getType() == TreeNodeType::BLOCK) {
            cout << "block" << blockCounter++ << ":" << endl;
            emitBlock(statement);
        }
        else {
            // todo other statements

        }
    }
}


void CodeGenerator::emitDeclaration(TreeNode* node) {
    Token token;
    for (int i = 0; i < node->getChildCount(); i++) {
        token = node->getChild(i)->getToken();
        cout << "iconst 0      // int ";
        cout.write(token.text, token.length);
        cout << ";" << endl;
    }
}

void CodeGenerator::emitCall(TreeNode* node) {
    for (int i = 0; i < node->getChildCount(); i++) {
        emitExpression(node->getChild(i));
    }
    cout << "call ";
    Token tkn = node->getToken();
    cout.write(tkn.text, tkn.length);
    cout << endl;
}


void CodeGenerator::emitIfElse(TreeNode* node) {
    TreeNode* condition = node->getChild(0);
    TreeNode* thenBlock = node->getChild(1);
    TreeNode* elseBlock = node->getChild(2);
    emitExpression(condition);
    cout << "ifeq  [ addr  ]" << endl;
    emitBlock(thenBlock);
    if (elseBlock != NULL) emitBlock(elseBlock);
    // address + length1;
}


void CodeGenerator::emitWhile(TreeNode* node) {

}

void CodeGenerator::emitReturn(TreeNode* node) {
    emitExpression(node->getChild(0));
    cout << "ret  ";
    cout << endl;
}


void CodeGenerator::emitAssignment(TreeNode* assignment) {
    Token asgn = assignment->getChild(0)->getToken();
    emitExpression(assignment->getChild(1));
    Symbol* entry = assignment->getSymbolTable()->lookupSymbol(asgn);
    if (entry != NULL) {
        cout << "istore #";
        cout << entry->localIndex;
        cout << endl;
    }
    else {

    }
}


void CodeGenerator::emitExpression(TreeNode* node) {
    size_t childCount = node->getChildCount();
    if (childCount == 0) {
        emitSymbol(node);
    } else if (node->getType() == TreeNodeType::BINARY_OP && childCount == 2) {
        emitExpression(node->getChild(0));
        emitExpression(node->getChild(1));
        getOpCode(node->getToken());
    } else if (node->getType() == TreeNodeType::CALL) {
        emitCall(node);
    } else {
        cout << "Error unknown Node" << endl;
    }
}


void CodeGenerator::emitSymbol(TreeNode* node) {
    Token token = node->getToken();
    if (node->getType() == TreeNodeType::SYMBOL) {
        Symbol* entry = node->getSymbolTable()->lookupSymbol(token);
        if (entry != NULL) {
            if (entry->type == SymbolType::ARGUMENT) {
                cout << "iarg  #";
                cout << entry->localIndex;
            }
            if (entry->type == SymbolType::VARIABLE) {
                cout << "iload #";
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


WORD CodeGenerator::getOpCode(Token& token) {
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

