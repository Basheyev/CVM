// cvm.cpp: определяет точку входа для приложения.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include "runtime/VirtualMachine.h"
#include "runtime/ExecutableImage.h"
#include "compiler/VMCompiler.h"
#include "compiler/VMNode.h"
#include "compiler/VMParser.h"
#include "compiler/VMCodeGenerator.h"

using namespace std;
using namespace vm;


//-------------------------------------------------------------------
// Virtual Machine Test
//-------------------------------------------------------------------
void createExecutableImage(ExecutableImage* img, WORD iterations) {
	
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
	img->emit(OP_JG, -11 );                     // if (ToS > 0) jump to [-11]           
	img->emit(OP_HALT);                         // end of program

	img->setEmitAddress(fn);                    // Function void fn()
	img->emit(OP_CONST, myStr);                 // Push constant string address
	img->emit(OP_SYSCALL, 0x20);                // Call system call 0x20, to print C style string to standard output
	img->emit(OP_RET);                          // Return
}



//-------------------------------------------------------------------
// Virtual Machine Test Paramters and Local variables
//-------------------------------------------------------------------
void createExecutableImage2(ExecutableImage* img, WORD iterations) {

	WORD dataSeg = 128;							// Data segment starts at 64
	WORD str = dataSeg;
	img->writeData(str, "Hello, world from VM!\n", 23);

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
	img->emit(OP_JG, -15);                      // if (ToS > 0) jump to -15   
	img->emit(OP_HALT);                         // end of program

	// int sum(a, b)
	img->setEmitAddress(sum);                   // int sum(a,b)
	img->emit(OP_ARG, 0);                       // load argment #0 (a)
	img->emit(OP_ARG, 1);                       // load argment #1 (b)
	img->emit(OP_ADD);                          // a+b	
	img->emit(OP_RET);                          // Return TOS

}


void vmTest() {
	ExecutableImage* img = new ExecutableImage();
	createExecutableImage2(img, 15);
	VirtualMachine* vm = new VirtualMachine();
	vm->loadImage(img->getImage(), img->getImageSize());
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
	ExecutableImage* image = new ExecutableImage();


	VMCompiler* compiler = new VMCompiler();
	compiler->compile("-3+5*(6+2)*(15-3)/5", image); // =93
	delete compiler;

	VirtualMachine* runtime = new VirtualMachine();
	runtime->loadImage(image->getImage(), image->getImageSize());
	runtime->execute();

	delete runtime;

	delete image;
}


//-------------------------------------------------------------------
// Node Test
//-------------------------------------------------------------------
bool loadFile(std::string& data, const char* filename)
{
	ios_base::openmode openmode = ios::ate | ios::in | ios::binary;
	ifstream file(filename, openmode);
	if (file.is_open()) {
		data.clear();
		streampos size = file.tellg();
		data.reserve(size);
		file.seekg(0, ios::beg);
		data.append(istreambuf_iterator<char>(file.rdbuf()), istreambuf_iterator<char>());
		file.close();
		return true;
	}
	return false;
}


void syntaxTreeTest() {
	string sourceCode;
	
	cout << filesystem::current_path() << endl;

	if (!loadFile(sourceCode, "c:/Learning/cvm/cvm/test/script00.cvm")) {
		cout << "File not open." << endl;
		return;
	}

	VMParser *tree = new VMParser();
	VMNode* root = tree->parse(sourceCode.c_str());
	if (root != NULL) root->print();
	delete tree;
}


void codeGeneratorTest() {
	string sourceCode;

	cout << filesystem::current_path() << endl;

	if (!loadFile(sourceCode, "c:/Learning/cvm/cvm/test/script01.cvm")) {
		cout << "File not open." << endl;
		return;
	}

	VMParser* parser = new VMParser();
	VMNode* root = parser->parse(sourceCode.c_str());
	if (root != NULL) {
		ExecutableImage* image = new ExecutableImage();
		VMCodeGenerator* codeGenerator = new VMCodeGenerator();
		// Print AST
		//root->print();
		// Generate code
		codeGenerator->generateCode(image, root);
		delete codeGenerator;
		delete image;
	}
	delete parser;

}


int main()
{
	vmTest();
	//lexerTest();
	//compilerTest();
	//syntaxTreeTest();
	//codeGeneratorTest();
	
	return 0;
}
