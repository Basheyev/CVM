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

// todo generate local variables at beginning
// todo error info and handling

constexpr WORD MAGIC_BREAK = 0xFFFFFFFF;
constexpr WORD JUMP_OFFSET = 2;


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
            // emit function code
            emitFunction(img, node);
        }
    }
}


 
void CodeGenerator::emitFunction(ExecutableImage* img, TreeNode* node) {
    // set function address in symbols table
    Token tkn = node->getToken();
    Symbol* symbol = node->getSymbolTable()->lookupSymbol(tkn);
    symbol->address = img->getEmitAddress();
    // Child nodes: #0 - return type, #1 - arguments, #2 - function body
    TreeNode* returnType = node->getChild(0);
    TreeNode* arguments = node->getChild(1);
    TreeNode* body = node->getChild(2);
    ExecutableImage funCode;

    // elevate all variable declaration to the function beginning
    emitDeclaration(&funCode, body);

    emitBlock(&funCode, body);
    // if no return statment then add return instruction;
    if (funCode.getSize() > 0) {
        WORD lastInstruction = funCode.readWord(funCode.getSize() - 1);
        if (lastInstruction != OP_RET) funCode.emit(OP_RET);
    } else funCode.emit(OP_RET);
    img->emit(funCode);

}



void CodeGenerator::emitStatement(ExecutableImage* img, TreeNode* statement) {
    switch (statement->getType()) {
    case TreeNodeType::TYPE:       break; // skip because already emitted
    case TreeNodeType::ASSIGNMENT: emitAssignment(img, statement); break;
    case TreeNodeType::IF_ELSE:    emitIfElse(img, statement); break;
    case TreeNodeType::WHILE:      emitWhile(img, statement); break;
    case TreeNodeType::CALL:       emitCall(img, statement); break;
    case TreeNodeType::BLOCK:      emitBlock(img, statement); break;
    case TreeNodeType::RETURN:     emitReturn(img, statement); break;
    case TreeNodeType::BREAK:      emitBreak(img, statement); break;
    default: raiseError("Unknown structure in syntax tree.");
    }
}


void CodeGenerator::emitBlock(ExecutableImage* img, TreeNode* body) {
    size_t count = body->getChildCount();
    TreeNode* statement;
    for (int j = 0; j < count; j++) {
        statement = body->getChild(j);
        emitStatement(img, statement);
    }
}


void CodeGenerator::emitDeclaration(ExecutableImage* img, TreeNode* node) {
    Token token;
    TreeNode* statement;
    // scan all child blocks and emit variable declaration
    for (int i = 0; i < node->getChildCount(); i++) {
        statement = node->getChild(i);
        if (node->getType() == TreeNodeType::TYPE) {
            for (int i = 0; i < node->getChildCount(); i++) {
                token = node->getChild(i)->getToken();
                img->emit(OP_CONST, 0);
            }
        } else if (node->getChildCount() > 0) {
            emitDeclaration(img, statement);
        }
    }
}


void CodeGenerator::emitCall(ExecutableImage* img, TreeNode* node) {
    
    // look up function name in symbols table
    Token funcToken = node->getToken();
    Symbol* func = node->getSymbolTable()->lookupSymbol(funcToken);
    if (func == NULL || func->type != SymbolType::FUNCTION) raiseError("Function not found.");

    // emit arguments expressions
    for (int i = 0; i < node->getChildCount(); i++) {
        emitExpression(img, node->getChild(i));
    }
        
    // todo make proper system call labeling in syntax tree
    // system function
    if (funcToken.length == 4 && strncmp(funcToken.text, "iput", 4) == 0) img->emit(OP_SYSCALL, 0x21); else
    if (funcToken.length == 4 && strncmp(funcToken.text, "iget", 4) == 0) img->emit(OP_SYSCALL, 0x22); 
    else {
        // user function
        WORD funcAddress = func->address;
        img->emit(OP_CALL, funcAddress, (WORD) node->getChildCount());
    }
}


void CodeGenerator::emitIfElse(ExecutableImage* img, TreeNode* node) {
    TreeNode* condition = node->getChild(0);
    TreeNode* thenBlock = node->getChild(1);
    TreeNode* elseBlock = node->getChild(2);
    ExecutableImage conditionCode, thenCode, elseCode;
    WORD offset;
    
    emitExpression(&conditionCode, condition);          // generate condition code
    emitStatement(&thenCode, thenBlock);                // generate then block code
    if (elseBlock) {                                    // if there is else block
        emitStatement(&elseCode, elseBlock);            // generate else block code
    }

    // IF: emit conditional jump code
    offset = thenCode.getSize() + 1;                    // calculate next address after then block 
    if (elseBlock) offset += JUMP_OFFSET;               // if there is an else block add JMP offset
    img->emit(conditionCode);
    img->emit(OP_IFZERO, offset);                      

    // THEN: emit then block code
    img->emit(thenCode);
    if (elseBlock) {                                    // if there is an else block
        offset = elseCode.getSize() + 1;                // calculate next address after else block
        img->emit(OP_JMP, offset);                      // emit unconditional jump over else block
    }

    // ELSE: emit else block code
    if (elseBlock) img->emit(elseCode);

}


void CodeGenerator::emitWhile(ExecutableImage* img, TreeNode* node) {
    TreeNode* condition = node->getChild(0);
    TreeNode* whileBlock = node->getChild(1);
    ExecutableImage conditionCode, whileCode;
    // Generate while block code
    emitStatement(&whileCode, whileBlock);     
    // Generate condition expression code
    emitExpression(&conditionCode, condition);     
    // Calculate next address after while block 
    WORD jumpOut = whileCode.getSize() + JUMP_OFFSET + 1; 
    
    // Emit coniditional jump
    img->emit(conditionCode);
    img->emit(OP_IFZERO, jumpOut);

    // Replace in while block code MAGIC_BREAK with relative jump out
    WORD whileCodeSize = whileCode.getSize();
    WORD w1, w2, offset;
    for (int i = 0; i < whileCodeSize - 1; i++) {
        w1 = whileCode.readWord(i);
        w2 = whileCode.readWord(i + 1);
        if (w1 == MAGIC_BREAK && w2 == MAGIC_BREAK) {
            offset = jumpOut - i - JUMP_OFFSET;
            whileCode.writeWord(i, OP_JMP);
            whileCode.writeWord(i + 1, offset);
        }
    }

    // Emit while block code to image
    img->emit(whileCode);                            
    // calculate relative offset to beginning of condition expression 
    WORD jumpBackOffset = -(whileCode.getSize() + conditionCode.getSize() + JUMP_OFFSET + 1);
    // Emit unconditional jump to 
    img->emit(OP_JMP, jumpBackOffset);      
}


void CodeGenerator::emitReturn(ExecutableImage* img, TreeNode* node) {
    emitExpression(img, node->getChild(0));
    img->emit(OP_RET);
}


void CodeGenerator::emitBreak(ExecutableImage* img, TreeNode* node) {
    // reserve space for jump out of While cycle and
    // mark BREAK as two MAGIC_BREAK, MAGIC_BREAK values
    // for later replacement
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
    TreeNodeType type = node->getType();
    Token token;
    string integerString;
    WORD value;

    switch (type) {
    case TreeNodeType::BINARY_OP:
        emitExpression(img, node->getChild(0));
        emitExpression(img, node->getChild(1));
        emitOpcode(img, node->getToken());
        break;
    case TreeNodeType::UNARY_OP:
        emitExpression(img, node->getChild(0));
        emitOpcode(img, node->getToken());
        break;
    case TreeNodeType::SYMBOL:
        emitSymbol(img, node);
        break;
    case TreeNodeType::CALL:
        emitCall(img, node);
        break;
    case TreeNodeType::CONSTANT:
        token = node->getToken();
        integerString.append(token.text, token.length);
        value = stoi(integerString);
        img->emit(OP_CONST, value);
        break;
    default:
        raiseError ("Error unknown abstract syntax tree node");
        break;
    }

}


void CodeGenerator::emitSymbol(ExecutableImage* img, TreeNode* node) {
    Token token = node->getToken();
    TreeNodeType type = node->getType();
    Symbol* entry;
    entry = node->getSymbolTable()->lookupSymbol(token);
    if (entry != NULL) {
        if (entry->type == SymbolType::ARGUMENT) img->emit(OP_ARG, entry->localIndex);
        else if (entry->type == SymbolType::VARIABLE) img->emit(OP_LOAD, entry->localIndex);
        else raiseError("Variable or argument expected.");
    } else raiseError("Symbol not declared.");
}


WORD CodeGenerator::emitOpcode(ExecutableImage* img, Token& token) {
    switch (token.type) {
    case TokenType::PLUS:      img->emit(OP_ADD);     break;
    case TokenType::MINUS:     img->emit(OP_SUB);     break;
    case TokenType::MULTIPLY:  img->emit(OP_MUL);     break;
    case TokenType::DIVIDE:    img->emit(OP_DIV);     break;
    case TokenType::EQUAL:     img->emit(OP_EQUAL);   break;
    case TokenType::NOT_EQUAL: img->emit(OP_NEQUAL);  break;
    case TokenType::GREATER:   img->emit(OP_GREATER); break;
    case TokenType::GR_EQUAL:  img->emit(OP_GREQUAL); break;
    case TokenType::LESS:      img->emit(OP_LESS);    break;
    case TokenType::LS_EQUAL:  img->emit(OP_LSEQUAL); break;
    case TokenType::LOGIC_AND: img->emit(OP_LAND);    break;
    case TokenType::LOGIC_OR:  img->emit(OP_LOR);     break;
    case TokenType::LOGIC_NOT: img->emit(OP_LNOT);    break;
    case TokenType::NOT:       img->emit(OP_NOT);     break;
    case TokenType::AND:       img->emit(OP_AND);     break;
    case TokenType::OR:        img->emit(OP_OR);      break;
    case TokenType::XOR:       img->emit(OP_XOR);     break;
    case TokenType::SHL:       img->emit(OP_SHL);     break;
    case TokenType::SHR:       img->emit(OP_SHR);     break;
    default:
        cout << "UNKNOWN OPERATION: ";
        cout.write(token.text, token.length);
        cout << endl;
        break;
    }
    return 0;
}

