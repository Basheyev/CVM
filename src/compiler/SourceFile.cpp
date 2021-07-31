/*============================================================================
*
*  Virtual Machine Compiler source code loader implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include <filesystem>
#include <fstream>

#include "compiler/SourceFile.h"

using namespace std;
using namespace vm;

SourceFile::SourceFile(const char* filename) {
	data = NULL;
	ios_base::openmode openmode = ios::ate | ios::in | ios::binary;
	ifstream file(filename, openmode);
	if (file.is_open()) {
		streampos pos = file.tellg();
		size_t size = pos;
		data = new char[size + 1];
		file.seekg(0, ios::beg);
		file.read(data, pos);
		file.close();
		// Terminate C string at the end
		data[size] = 0;  
	}
}

SourceFile::~SourceFile() {
	if (data != NULL) {
		delete[] data;
		data = NULL;
	}
}

char* SourceFile::getData() {
	return data;
}
