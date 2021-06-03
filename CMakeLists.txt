# CMakeList.txt: проект CMake для cvm; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("cvm")

include_directories("include")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (cvm "src/cvm.cpp" "src/VMRuntime.cpp"   "include/VMRuntime.h"  "include/VMCompiler.h" "src/VMCompiler.cpp" "include/VMImage.h" "src/VMImage.cpp" "src/VMParser.cpp" "include/VMParser.h")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
