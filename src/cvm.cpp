// cvm.cpp: определяет точку входа для приложения.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include "runtime/VMRuntime.h"
#include "image/VMImage.h"
#include "compiler/VMCompiler.h"
#include "compiler/VMNode.h"
#include "compiler/VMSyntaxTree.h"

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

	VMSyntaxTree *tree = new VMSyntaxTree();
	VMNode* root = tree->parse(sourceCode.c_str());
	if (root != NULL) root->print();
	delete tree;
}


int main()
{

	//vmTest();
	
	//lexerTest();
	
	//compilerTest();

	syntaxTreeTest();

	return 0;
}
