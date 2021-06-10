// cvm.cpp: определяет точку входа для приложения.
//

#include <iostream>
#include <fstream>
#include <chrono>
#include "runtime/VMRuntime.h"
#include "image/VMImage.h"
#include "compiler/VMCompiler.h"
#include "compiler/VMNode.h"

using namespace std;
using namespace vm;


//-------------------------------------------------------------------
// Virtual Machine Test
//-------------------------------------------------------------------
void createExecutableImage(VMImage* img, WORD iterations) {
	
	WORD dataSeg = 32;							// Data segment starts at 256
	
	WORD iVar = dataSeg;
	WORD myStr = dataSeg + 1;
	img->writeWord(iVar, iterations);
	img->writeData(myStr, "Hello, world from VM!\n", 23);    
	
	WORD fn = 16;

	WORD addr = img->emit(OP_PUSH, iVar);       // stack <- [iVar] (operand 1)
	img->emit(OP_DEC);                          // stack[top]--  (operand 1 decrement)
	img->emit(OP_CALL, fn);                     // Call function fn()     
	img->emit(OP_DUP);                          // duplicate stack top (operand 1 duplicate)
	img->emit(OP_POP, iVar);                    // stack -> [iVar] (pop operand 1 duplicate to iVar)
	img->emit(OP_CONST, 0);                     // push const 0 (operand 2)
	img->emit(OP_CMPJG, addr);                  // if (operand1 > operand2) jump to addr           
	img->emit(OP_HALT);                         // end of program


	img->setEmitPointer(fn);                    // Function fn()
	img->emit(OP_CONST, myStr);                 // Push constant string address
	img->emit(OP_SYSCALL, 0x20);                // Call system call 0x20, to print C style string to standard output
	img->emit(OP_RET);                          // Return
}


void vmTest() {
	VMImage* img = new VMImage();
	createExecutableImage(img, 10);
	VMRuntime* vm = new VMRuntime();
	vm->loadImage(img->getImage(), img->getImageSize());
	img->dissasemble();
	cout << "-----------------------------------------------------" << endl;
	cout << "Virtual Machine test:" << endl;
	cout << "-----------------------------------------------------" << endl;
	auto start = std::chrono::high_resolution_clock::now();
	vm->run();
	auto end = std::chrono::high_resolution_clock::now();
	auto ms_int = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
	cout << "EXECUTION TIME: " << ms_int / 1000000000.0 << "s" << endl;
	delete vm;
	delete img;
}


//-------------------------------------------------------------------
// Lexer Test
//-------------------------------------------------------------------

void lexerTest() {
	VMLexer* lexer = new VMLexer();
	char* sourceCode = "int main()\n"
		"{\n"
		"\t  printf (\"Wow!\");\n"
		"\t  float a = 365.0 * 10 - 10.0 / 2 + 3;\n"
		"\t  while (1 != 2) {\n"
		"\t      abc.v1 = 'x';\n"
		"\t  };\n"
		"\t  if (a >= b) return a && b; else a || b;\n" 
		"}\n";
	lexer->parseToTokens(sourceCode);
	lexer->printAllTokens();
	delete lexer;
}

//-------------------------------------------------------------------
// Compiler Test
//-------------------------------------------------------------------
void compilerTest() {
	VMImage* image = new VMImage();


	VMCompiler* compiler = new VMCompiler();
	compiler->compile("-3+5*(6+2)*(15-3)/5", image);
	delete compiler;

	VMRuntime* runtime = new VMRuntime();
	runtime->loadImage(image->getImage(), image->getImageSize());
	runtime->run();

	delete runtime;

	delete image;
}


//-------------------------------------------------------------------
// Node Test
//-------------------------------------------------------------------
void nodeTest() {
	VMNode* root = new VMNode(NULL,NULL);

	Token ta = { TokenType::CONST_CHAR, "'a'", 3, 1, 1 };
	Token tb = { TokenType::CONST_CHAR, "'b'", 3, 1, 1 };
	Token tc = { TokenType::CONST_CHAR, "'c'", 3, 1, 1 };
	Token td = { TokenType::CONST_CHAR, "'d'", 3, 1, 1 };
	Token te = { TokenType::CONST_CHAR, "'e'", 3, 1, 1 };
	Token tf = { TokenType::CONST_CHAR, "'f'", 3, 1, 1 };

	VMNode* a = root->addChild(ta);
	VMNode* b = a->addChild(tb);
	VMNode* c = a->addChild(tc);
	VMNode* d = b->addChild(td);
	VMNode* e = c->addChild(te);
	VMNode* f = c->addChild(tf);

	root->print();

	a->removeChild(b);
	c->removeChild(f);

	cout << endl;
	root->print();

	delete root;
}


int main()
{

	//vmTest();
	
	//lexerTest();
	
	//compilerTest();

	nodeTest();

	return 0;
}
