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


// todo refactor it
void compileRun(string filepath, bool showTree, bool showSymbols, bool disassemble, bool run) {

	// Read source code file
	SourceFile source(filepath.c_str());
	cout << "Current path: " << filesystem::current_path() << endl;
	if (source.getData()==NULL) {
		cout << "File not open." << endl;
		return;
	}
	
	// Parse source code
	ExecutableImage* img = new ExecutableImage();
	SourceParser* parser = new SourceParser(source.getData());
	TreeNode *root = parser->getSyntaxTree();
	if (root == NULL) {
		cout << "Parser error. Can not parse source code.";
		delete parser;
		return;
	} else {
		if (showTree) root->print();
		// Generate executable image
		CodeGenerator* codeGenerator = new CodeGenerator();
		if (!codeGenerator->generateCode(img, parser->getSyntaxTree())) {
			cout << "Code generator error. Can not generate code.";
			delete codeGenerator;
			delete img;
			return;
		} else {
			if (showSymbols) parser->getSymbolTable().printSymbols();
			if (disassemble) img->disassemble();
			delete codeGenerator;
			delete parser;
		}
	}

	;
	
	// Run executable image
	if (run) {
		VirtualMachine* machine = new VirtualMachine();
		machine->loadImage(*img);
		auto start = std::chrono::high_resolution_clock::now();
		machine->execute();
		auto end = std::chrono::high_resolution_clock::now();
		auto ms_int = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
		cout << "Execution time: " << ms_int / 1000000000.0 << "s" << endl;
		delete machine;
	}
	
	delete img;

}


int main()
{
	//compileRun("../../../test/factorial.cvm", true, true, true, true);
	compileRun("../../../test/primenumber.cvm", true, true, true, true);
	//compileRun("../../../test/combinatorics.cvm", true, true, true, true);
	//compileRun("../../../test/scope.cvm", true, true, true, true);
	return 0;
}
