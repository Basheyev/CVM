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
#include <cstring>


using namespace vm;
using namespace std;


CodeGenerator::CodeGenerator() {

}

CodeGenerator::~CodeGenerator() {


}


void CodeGenerator::generateCode(ExecutableImage* img, TreeNode* rootNode) {
    try {
        img->clear();                        // clear executable image
        img->setEmitAddress(4);              // reserve 4 memory cells to call main() entry point
        emitModule(img, rootNode);           // emit module code starting from address [4]
        // Lookup entry point address
        SymbolTable* global = rootNode->getSymbolTable();
        Symbol* main = global->lookupSymbol("main", SymbolType::FUNCTION);
        if (main == NULL) {
            raiseError("No entry point found - int main() function missing.");
        } else {
            // emit uncoditional jump to entry point;
            img->setEmitAddress(0);
            img->emit(OP_CALL, main->address, 0);
            img->emit(OP_HALT);
        }
    }
    catch (CodeGeneratorException& e) {
        cout << "CODE GENERATION ERROR: ";
        cout << e.error << endl;
    }
}


void CodeGenerator::emitModule(ExecutableImage* img, TreeNode* rootNode) {
    // TODO Sort data and functions (.text, .data)
    for (int i = 0; i < rootNode->getChildCount(); i++) {
        TreeNode* node = rootNode->getChild(i);
        if (node->getType() == TreeNodeType::FUNCTION) {
            Token tkn = node->getToken();
            cout << endl;
            cout.write(tkn.text, tkn.length);
            cout << ":" << endl;

            // emit function code and get address
            ExecutableImage function;
            emitFunction(&function, node);
            WORD funcAddress = img->getEmitAddress();
            img->emit(function);

            // get funcation address
            Symbol* symbol = node->getSymbolTable()->lookupSymbol(tkn);
            symbol->address = funcAddress;

        } else if (node->getType() == TreeNodeType::TYPE) {
            ExecutableImage data;
            emitDeclaration(&data, node);
            // emit data to image
        }
    }
}


// Node Childs: #0 - type, #1 - arguments, #2 - function body
void CodeGenerator::emitFunction(ExecutableImage* img, TreeNode* node) {
    //Token name = node->getToken();
    TreeNode* returnType = node->getChild(0);
    TreeNode* arguments = node->getChild(1);
    TreeNode* body = node->getChild(2);
    emitBlock(img, body);
}


void CodeGenerator::emitBlock(ExecutableImage* img, TreeNode* body) {
    // emit function body
    for (int j = 0; j < body->getChildCount(); j++) {
        TreeNode* statement = body->getChild(j);
        if (statement->getType() == TreeNodeType::TYPE) emitDeclaration(img, statement);
        else if (statement->getType() == TreeNodeType::ASSIGNMENT) emitAssignment(img, statement);
        else if (statement->getType() == TreeNodeType::RETURN) emitReturn(img, statement);
        else if (statement->getType() == TreeNodeType::IF_ELSE) emitIfElse(img, statement);
        else if (statement->getType() == TreeNodeType::BLOCK) {
            cout << "block" << body->getSymbolTable()->getName() << ":" << endl;
            emitBlock(img, statement);
        }
        else {
            // todo other statements

        }
    }
}


void CodeGenerator::emitDeclaration(ExecutableImage* img, TreeNode* node) {
    Token token;
    for (int i = 0; i < node->getChildCount(); i++) {
        token = node->getChild(i)->getToken();
        img->emit(OP_CONST, 0);

        cout << "iconst 0      // int ";
        cout.write(token.text, token.length);
        cout << ";" << endl;
    }
}


void CodeGenerator::emitCall(ExecutableImage* img, TreeNode* node) {
    for (int i = 0; i < node->getChildCount(); i++) {
        emitExpression(img, node->getChild(i));
    }

    Token funcToken = node->getToken();
    Symbol* func = node->getSymbolTable()->lookupSymbol(funcToken);
    if (func == NULL || func->type != SymbolType::FUNCTION) {
        raiseError("Function not found.");
        return;
    }
    WORD funcAddress = func->address;
    img->emit(OP_CALL, funcAddress, node->getChildCount());

    cout << "call [" << funcAddress << "], " << node->getChildCount() << endl;

}


void CodeGenerator::emitIfElse(ExecutableImage* img, TreeNode* node) {
    TreeNode* condition = node->getChild(0);
    TreeNode* thenBlock = node->getChild(1);
    TreeNode* elseBlock = node->getChild(2);
    ExecutableImage thenCode, elseCode;
    // condition
    emitExpression(img, condition);
    // if
    emitBlock(&thenCode, thenBlock);
    img->emit(OP_IFNE, thenCode.getImageSize() + 1);
    // then
    img->emit(thenCode);
    // else
    if (elseBlock != NULL) {
        emitBlock(&elseCode, elseBlock);
        img->emit(OP_JMP, elseCode.getImageSize() + 1);
        img->emit(elseCode);
    }

    cout << "ifeq  [+" << elseCode.getImageSize() + 1 << "]" << endl;
}


void CodeGenerator::emitWhile(ExecutableImage* img, TreeNode* node) {

}

void CodeGenerator::emitReturn(ExecutableImage* img, TreeNode* node) {
    emitExpression(img, node->getChild(0));
    img->emit(OP_RET);
    cout << "ret  ";
    cout << endl;
}


void CodeGenerator::emitAssignment(ExecutableImage* img, TreeNode* assignment) {
    Token asgn = assignment->getChild(0)->getToken();
    emitExpression(img, assignment->getChild(1));
    Symbol* entry = assignment->getSymbolTable()->lookupSymbol(asgn);
    if (entry != NULL) {
        img->emit(OP_STORE, entry->localIndex);
        cout << "istore #";
        cout << entry->localIndex;
        cout << endl;
    }
    else {

    }
}


void CodeGenerator::emitExpression(ExecutableImage* img, TreeNode* node) {
    size_t childCount = node->getChildCount();
    if (childCount == 0) {
        emitSymbol(img, node);
    } else if (node->getType() == TreeNodeType::BINARY_OP && childCount == 2) {
        emitExpression(img, node->getChild(0));
        emitExpression(img, node->getChild(1));
        getOpCode(img, node->getToken());
    } else if (node->getType() == TreeNodeType::CALL) {
        emitCall(img, node);
    } else {
        cout << "Error unknown Node" << endl;
    }
}


void CodeGenerator::emitSymbol(ExecutableImage* img, TreeNode* node) {
    Token token = node->getToken();
    if (node->getType() == TreeNodeType::SYMBOL) {
        Symbol* entry = node->getSymbolTable()->lookupSymbol(token);
        if (entry != NULL) {
            if (entry->type == SymbolType::ARGUMENT) {
                img->emit(OP_ARG, entry->localIndex);
                cout << "iarg  #";
                cout << entry->localIndex;
            }
            if (entry->type == SymbolType::VARIABLE) {
                img->emit(OP_LOAD, entry->localIndex);
                cout << "iload #";
                cout << entry->localIndex;
            }
        }
    }
    else {
        string str;
        str.append(token.text, token.length);
        WORD value = stoi(str);
        img->emit(OP_CONST, value);
        cout << "iconst ";
        cout.write(token.text, token.length);
    }
    cout << endl;
}


WORD CodeGenerator::getOpCode(ExecutableImage* img, Token& token) {
    switch (token.type) {
    case TokenType::PLUS: 
        img->emit(OP_ADD);
        cout << "iadd" << endl; 
        break;
    case TokenType::MINUS: 
        img->emit(OP_SUB);
        cout << "isub" << endl; 
        break;
    case TokenType::MULTIPLY: 
        img->emit(OP_MUL);
        cout << "imul" << endl; 
        break;
    case TokenType::DIVIDE: 
        img->emit(OP_DIV);
        cout << "idiv" << endl; 
        break;
    default:
        cout << "UNKNOWN BINARY OPERATION: ";
        cout.write(token.text, token.length);
        cout << endl;
        break;
    }
    return 0;
}

