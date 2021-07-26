# CMakeList.txt: проект CMake для cvm; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("cvm")

include_directories("include" "include/runtime" "include/compiler")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (cvm 
	"include/runtime/VirtualMachine.h"  
	"include/runtime/ExecutableImage.h"
	"include/compiler/CodeGenerator.h"
	"include/compiler/SourceParser.h" 
	
	"src/cvm.cpp" 
	"src/runtime/VirtualMachine.cpp"   
	"src/runtime/ExecutableImage.cpp"
	"src/compiler/CodeGenerator.cpp"
	"src/compiler/SymbolTable.cpp"
	"src/compiler/SourceParser.cpp" 
	"src/compiler/TreeNode.cpp")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
target_compile_features(cvm PUBLIC cxx_std_17)