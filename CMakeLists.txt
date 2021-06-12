# CMakeList.txt: проект CMake для cvm; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("cvm")

include_directories("include" "include/runtime" "include/image" "include/compiler")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (cvm 
	"include/runtime/VMRuntime.h"  
	"include/image/VMImage.h" 
	"include/compiler/VMCompiler.h" 
	"include/compiler/VMLexer.h" 
	"include/compiler/VMNode.h"
	"include/compiler/VMSyntaxTree.h" 

	"src/cvm.cpp" 
	"src/runtime/VMRuntime.cpp"   
	"src/image/VMImage.cpp" 
	"src/compiler/VMCompiler.cpp" 
	"src/compiler/VMLexer.cpp"
	"src/compiler/VMNode.cpp"
	"src/compiler/VMSyntaxTree.cpp")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
target_compile_features(cvm PUBLIC cxx_std_17)