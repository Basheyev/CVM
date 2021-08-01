// cvm.cpp: определяет точку входа для приложения.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>

#include "runtime/VirtualMachine.h"
#include "compiler/SourceParser.h"
#include "compiler/CodeGenerator.h"
#include "compiler/SourceFile.h"


using namespace std;
using namespace vm;


//-------------------------------------------------------------------
// Virtual Machine Test
//-------------------------------------------------------------------
void buildImage1(ExecutableImage* img, WORD iterations) {
	
	WORD dataSeg = 64;							// Data segment starts at 64
	
	WORD iVar = dataSeg;
	WORD myStr = dataSeg + 1;
	img->writeWord(iVar, iterations);
	img->writeData(myStr, "Hello, world from VM!\n", 23);    
	
	WORD fn = 32;

	WORD addr = img->emit(OP_PUSH, iVar);       // stack <- [iVar] (operand 1)
	img->emit(OP_DEC);                          // stack[top]--  (operand 1 decrement)
	img->emit(OP_DUP);                          // duplicate stack top (operand 1 duplicate)
	img->emit(OP_POP, iVar);                    // stack -> [iVar] (pop operand 1 duplicate to iVar)
	img->emit(OP_CALL, fn, 0);                  // Call function void fn()
	img->emit(OP_DROP);                         // Drop return value if void
	img->emit(OP_IFGR, -11 );                     // if (ToS > 0) jump to [-11]           
	img->emit(OP_HALT);                         // end of program

	img->setEmitAddress(fn);                    // Function void fn()
	img->emit(OP_CONST, myStr);                 // Push constant string address
	img->emit(OP_SYSCALL, 0x20);                // Call system call 0x20, to print C style string to standard output
	img->emit(OP_RET);                          // Return
}



//-------------------------------------------------------------------
// Virtual Machine Test Paramters and Local variables
//-------------------------------------------------------------------
void buildImage2(ExecutableImage* img, WORD iterations) {

	WORD dataSeg = 128;							// Data segment starts at 64

	WORD sum = 32;
	WORD hello = 64;

	img->emit(OP_CONST, iterations);            // initialize local variable #0
	WORD addr = img->emit(OP_LOAD, 0);          // push to stack local variable #0
	img->emit(OP_DEC);                          // stack[top]--  (operand 1 decrement)
	img->emit(OP_DUP);                          // duplicate stack top (operand 1 duplicate)
	img->emit(OP_STORE, 0);                     // load top of stack to local variable #0
	img->emit(OP_DUP);                          // duplicate stack top (operand 1 duplicate)
	img->emit(OP_CONST, 10);                    // push const
	img->emit(OP_CALL, sum, 2);                 // Call function fn(a, b)  
	img->emit(OP_SYSCALL, 0x21);                // print TOS int
	img->emit(OP_IFGR, -15);                      // if (ToS > 0) jump to -15   
	img->emit(OP_HALT);                         // end of program

	// int sum(a, b)
	img->setEmitAddress(sum);                   // int sum(a,b)
	img->emit(OP_ARG, 0);                       // load argment #0 (a)
	img->emit(OP_ARG, 1);                       // load argment #1 (b)
	img->emit(OP_ADD);                          // a+b	
	img->emit(OP_RET);                          // Return TOS

}


void vmTest(int num) {
	ExecutableImage* img = new ExecutableImage();
	if (num == 1) buildImage1(img, 15); else buildImage2(img, 15);
	VirtualMachine* vm = new VirtualMachine();
	vm->loadImage(*img);
	img->disassemble();
	cout << "-----------------------------------------------------" << endl;
	cout << "Virtual Machine test:" << endl;
	cout << "-----------------------------------------------------" << endl;
	auto start = std::chrono::high_resolution_clock::now();
	vm->execute();
	auto end = std::chrono::high_resolution_clock::now();
	auto ms_int = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
	cout << "EXECUTION TIME: " << ms_int / 1000000000.0 << "s" << endl;
	delete vm;
	delete img;
}



void sourceParserTest() {
	SourceFile source("../../../test/script00.cvm");
	cout << filesystem::current_path() << endl;
	if (source.getData()==NULL) {
		cout << "File not open." << endl;
		return;
	}
	
	auto start = std::chrono::high_resolution_clock::now();
	SourceParser* parser = new SourceParser(source.getData());
	auto end = std::chrono::high_resolution_clock::now();
	auto ms_int = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
	cout << "EXECUTION TIME: " << ms_int / 1000000000.0 << "s" << endl;
	TreeNode *root = parser->getSyntaxTree();
	ExecutableImage* img = new ExecutableImage();
	CodeGenerator *codeGenerator = new CodeGenerator();
	codeGenerator->generateCode(img, parser->getSyntaxTree());
	if (root != NULL) {
		root->print();
		parser->getSymbolTable().printSymbols();
	}
	img->disassemble();

	VirtualMachine* machine = new VirtualMachine();
	machine->loadImage(*img);
	machine->execute();
	delete machine;
	delete img;
	delete codeGenerator;
	delete parser;
}


int main()
{
	//vmTest(1);
	// vmTest(2);
	 sourceParserTest();

	return 0;
}
