// cvm.cpp: определяет точку входа для приложения.
//

#include <iostream>
#include <fstream>
#include <chrono>
#include "VMRuntime.h"
#include "VMImage.h"
#include "VMCompiler.h"

using namespace std;
using namespace vm;


void createExecutableImage(VMImage* img, WORD iterations) {
	
	WORD dataSeg = 256;							// Data segment starts at 256
	
	WORD iVar = dataSeg;
	WORD myStr = dataSeg + 1;
	img->writeWord(iVar, iterations);
	img->writeData(myStr, "Hello, world from VM!\n", 23);    
	
	WORD fn = 128;

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


void opcodesTest() {
	VMImage* img = new VMImage();
	createExecutableImage(img, 10);
	VMRuntime* vm = new VMRuntime();
	vm->loadImage(img->getImage(), img->getImageSize());
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


void parserTest() {
	VMLexer* parser = new VMLexer();
	char* sourceCode = "int main()\n"
		"{\n"
		"\t  printf (\"Wow!\");\n"
		"\t  float a = 365.0 * 10 - 10.0 / 2 + 3;\n"
		"\t  while (1 != 2) {\n"
		"\t      abc.v1 = 'x';\n"
		"\t  };\n"
		"\t  if (a >= b) return a && b; else a || b;\n" 
		"}\n";
	parser->parseToTokens(sourceCode);
	parser->printAllTokens();
	delete parser;
}



void compilerTest() {
	VMImage* image = new VMImage();


	VMCompiler* compiler = new VMCompiler();
	compiler->compile("3+5*6+2*3+15/5", image);
	delete compiler;

	VMRuntime* runtime = new VMRuntime();
	runtime->loadImage(image->getImage(), image->getImageSize());
	runtime->run();

	delete runtime;

	delete image;
}


int main()
{

	//opcodesTest();
	
	//parserTest();
	
	compilerTest();

	return 0;
}
