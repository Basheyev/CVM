/*============================================================================
*
*  Virtual Machine Compiler code generator imlementation
*
*  (C)Bolat Basheyev 2021
*
============================================================================*/

#include "compiler/CodeGenerator.h"

#include <iostream>
#include <cstring>


using namespace vm;
using namespace std;

constexpr WORD MAGIC_BREAK = 0xFFFFFFFF;

// todo generate local variables at beginning
// todo add break;
// todo error info and handling

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
        if (main == NULL || main->argCount != 0) {
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
            // set function address
            Token tkn = node->getToken();
            Symbol* symbol = node->getSymbolTable()->lookupSymbol(tkn);
            symbol->address = img->getEmitAddress();
            // emit function code
            emitFunction(img, node);
        } else if (node->getType() == TreeNodeType::TYPE) {
            // emit global variables to image
            ExecutableImage data;
            emitDeclaration(&data, node);
            // todo global variables ...            
        }
    }
}


// Node Childs: #0 - type, #1 - arguments, #2 - function body
void CodeGenerator::emitFunction(ExecutableImage* img, TreeNode* node) {
    //Token name = node->getToken();
    TreeNode* returnType = node->getChild(0);
    TreeNode* arguments = node->getChild(1);
    TreeNode* body = node->getChild(2);
    ExecutableImage funCode;
    emitBlock(&funCode, body);
    // if no return statment then add return instruction;
    if (funCode.getSize() > 0) {
        WORD lastInstruction = funCode.readWord(funCode.getSize() - 1);
        if (lastInstruction != OP_RET) funCode.emit(OP_RET);
    } else funCode.emit(OP_RET);
    img->emit(funCode);

}



void CodeGenerator::emitStatement(ExecutableImage* img, TreeNode* statement) {
    if (statement->getType() == TreeNodeType::TYPE) emitDeclaration(img, statement);
    else if (statement->getType() == TreeNodeType::ASSIGNMENT) emitAssignment(img, statement);
    else if (statement->getType() == TreeNodeType::RETURN) emitReturn(img, statement);
    else if (statement->getType() == TreeNodeType::BREAK) emitBreak(img, statement);
    else if (statement->getType() == TreeNodeType::IF_ELSE) emitIfElse(img, statement);
    else if (statement->getType() == TreeNodeType::WHILE) emitWhile(img, statement);
    else if (statement->getType() == TreeNodeType::CALL) emitCall(img, statement);
    else if (statement->getType() == TreeNodeType::BLOCK) emitBlock(img, statement);
    else raiseError("Unknown structure in syntax tree.");
}


void CodeGenerator::emitBlock(ExecutableImage* img, TreeNode* body) {
    for (int j = 0; j < body->getChildCount(); j++) {
        TreeNode* statement = body->getChild(j);
        emitStatement(img, statement);
    }
}


void CodeGenerator::emitDeclaration(ExecutableImage* img, TreeNode* node) {
    Token token;
    // TODO allocate all space in the beginning (?)
    for (int i = 0; i < node->getChildCount(); i++) {
        token = node->getChild(i)->getToken();
        img->emit(OP_CONST, 0);
    }
}


void CodeGenerator::emitCall(ExecutableImage* img, TreeNode* node) {
    for (int i = 0; i < node->getChildCount(); i++) {
        emitExpression(img, node->getChild(i));
    }

    Token funcToken = node->getToken();
    // system function
    if (funcToken.length == 4 && strncmp(funcToken.text, "iput", 4) == 0) img->emit(OP_SYSCALL, 0x21); else
    if (funcToken.length == 4 && strncmp(funcToken.text, "iget", 4) == 0) img->emit(OP_SYSCALL, 0x22); 
    else {
        // user function
        Symbol* func = node->getSymbolTable()->lookupSymbol(funcToken);
        if (func == NULL || func->type != SymbolType::FUNCTION) {
            raiseError("Function not found.");
            return;
        }
        WORD funcAddress = func->address;
        img->emit(OP_CALL, funcAddress, (WORD) node->getChildCount());
    }
}


void CodeGenerator::emitIfElse(ExecutableImage* img, TreeNode* node) {
    TreeNode* condition = node->getChild(0);
    TreeNode* thenBlock = node->getChild(1);
    TreeNode* elseBlock = node->getChild(2);
    ExecutableImage thenCode, elseCode;
    // condition
    emitExpression(img, condition);
    // if
    emitStatement(&thenCode, thenBlock);                // generate then block code
    WORD offset = thenCode.getSize() + 1;
    if (elseBlock != NULL) offset += 2;                 // +2 word of else skip jump ops
    img->emit(OP_IFZERO,  offset);                      // +1 operand 
    // then
    img->emit(thenCode);
    // else
    if (elseBlock != NULL) {
        emitStatement(&elseCode, elseBlock);
        img->emit(OP_JMP, elseCode.getSize() + 1);      // +1 operand +2 else skip jump
        img->emit(elseCode);
    }
}


void CodeGenerator::emitWhile(ExecutableImage* img, TreeNode* node) {
    TreeNode* condition = node->getChild(0);
    TreeNode* whileBlock = node->getChild(1);
    ExecutableImage conditionCode, whileCode;
    // Generate while block code
    emitStatement(&whileCode, whileBlock);
    // Generate and emit condition expression code
    emitExpression(&conditionCode, condition);
    img->emit(conditionCode);
    // +1 operand, +2 jmp [offset]
    WORD jumpOut = whileCode.getSize() + 1 + 2;
    img->emit(OP_IFZERO, jumpOut);

    // search BREAK statements and replace with relative jump out
    WORD w1, w2, offset;
    for (int i = 0; i < whileCode.getSize() - 1; i++) {
        w1 = whileCode.readWord(i);
        w2 = whileCode.readWord(i + 1);
        if (w1 == MAGIC_BREAK && w2 == MAGIC_BREAK) {
            offset = jumpOut - i - 1 - 2;
            whileCode.writeWord(i, OP_JMP);
            whileCode.writeWord(i + 1, offset);
        }
    }

    // Emit while block statements
    img->emit(whileCode);                            
    // unconditional jump back to condition expression 
    img->emit(OP_JMP, -whileCode.getSize() - conditionCode.getSize() - 1 - 2);      
}

void CodeGenerator::emitReturn(ExecutableImage* img, TreeNode* node) {
    emitExpression(img, node->getChild(0));
    img->emit(OP_RET);
}

void CodeGenerator::emitBreak(ExecutableImage* img, TreeNode* node) {
    // reserve space for jump out of While cycle and
    // mark BREAK as two MAGIC_BREAK, MAGIC_BREAK values
    img->emit(MAGIC_BREAK, MAGIC_BREAK);
}


void CodeGenerator::emitAssignment(ExecutableImage* img, TreeNode* assignment) {
    Token asgn = assignment->getChild(0)->getToken();
    emitExpression(img, assignment->getChild(1));
    Symbol* entry = assignment->getSymbolTable()->lookupSymbol(asgn);
    if (entry != NULL && entry->type==SymbolType::VARIABLE) {
        img->emit(OP_STORE, entry->localIndex);
    } else {
        raiseError("Can not assign if its not variable.");
    }
}


void CodeGenerator::emitExpression(ExecutableImage* img, TreeNode* node) {
    size_t childCount = node->getChildCount();
    if (node->getType() == TreeNodeType::SYMBOL) {
        emitSymbol(img, node);
    }
    else if (node->getType() == TreeNodeType::CONSTANT) {
        Token token = node->getToken();
        string str;
        str.append(token.text, token.length);
        WORD value = stoi(str);
        img->emit(OP_CONST, value);
    }
    else if (node->getType() == TreeNodeType::BINARY_OP && childCount == 2) {
        emitExpression(img, node->getChild(0));
        emitExpression(img, node->getChild(1));
        getOpCode(img, node->getToken());
    }
    else if (node->getType() == TreeNodeType::UNARY_OP && childCount == 1) {
        emitExpression(img, node->getChild(0));
        getOpCode(img, node->getToken());
    } else if (node->getType() == TreeNodeType::CALL) {
        emitCall(img, node);
    } else {
        cout << "Error unknown Node" << endl;
    }
}


void CodeGenerator::emitSymbol(ExecutableImage* img, TreeNode* node) {
    Token token = node->getToken();
    TreeNodeType type = node->getType();
    if (type == TreeNodeType::SYMBOL) {
        Symbol* entry = node->getSymbolTable()->lookupSymbol(token);
        if (entry != NULL) {
            if (entry->type == SymbolType::ARGUMENT) {
                img->emit(OP_ARG, entry->localIndex);
            }
            if (entry->type == SymbolType::VARIABLE) {
                img->emit(OP_LOAD, entry->localIndex);
            }
        }
    } 
}


WORD CodeGenerator::getOpCode(ExecutableImage* img, Token& token) {
    switch (token.type) {
    case TokenType::PLUS:      img->emit(OP_ADD);  break;
    case TokenType::MINUS:     img->emit(OP_SUB);  break;
    case TokenType::MULTIPLY:  img->emit(OP_MUL);  break;
    case TokenType::DIVIDE:    img->emit(OP_DIV);  break;
    case TokenType::EQUAL:     img->emit(OP_EQ);   break;
    case TokenType::NOT_EQUAL: img->emit(OP_NE);   break;
    case TokenType::GREATER:   img->emit(OP_GR);   break;
    case TokenType::GR_EQUAL:  img->emit(OP_GE);   break;
    case TokenType::LESS:      img->emit(OP_LS);   break;
    case TokenType::LS_EQUAL:  img->emit(OP_LE);   break;
    case TokenType::LOGIC_AND: img->emit(OP_LAND); break;
    case TokenType::LOGIC_OR:  img->emit(OP_LOR);  break;
    case TokenType::LOGIC_NOT: img->emit(OP_LNOT); break;
    case TokenType::NOT:       img->emit(OP_NOT);  break;
    case TokenType::AND:       img->emit(OP_AND);  break;
    case TokenType::OR:        img->emit(OP_OR);   break;
    case TokenType::XOR:       img->emit(OP_XOR);  break;
    case TokenType::SHL:       img->emit(OP_SHL);  break;
    case TokenType::SHR:       img->emit(OP_SHR);  break;
    default:
        cout << "UNKNOWN BINARY OPERATION: ";
        cout.write(token.text, token.length);
        cout << endl;
        break;
    }
    return 0;
}

