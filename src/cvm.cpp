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



void compilerTest() {
	SourceFile source("../../../test/script00.cvm");
	cout << filesystem::current_path() << endl;
	if (source.getData()==NULL) {
		cout << "File not open." << endl;
		return;
	}
	
	
	SourceParser* parser = new SourceParser(source.getData());
	TreeNode *root = parser->getSyntaxTree();

	if (root == NULL) return;

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
	auto start = std::chrono::high_resolution_clock::now();
	machine->execute();
	auto end = std::chrono::high_resolution_clock::now();
	auto ms_int = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
	cout << "EXECUTION TIME: " << ms_int / 1000000000.0 << "s" << endl;
	delete machine;
	delete img;
	delete codeGenerator;
	delete parser;
}


int main()
{
	compilerTest();

	return 0;
}
