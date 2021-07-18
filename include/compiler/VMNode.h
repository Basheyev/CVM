/*============================================================================
*
*  Abstract Syntax Tree Node class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/

#pragma once

#include "compiler/VMLexer.h"
#include <vector>

namespace vm {

	enum class VMNodeType { 
		UNKNOWN = 0, CONSTANT, ARGUMENTS, SYMBOL, DATA_TYPE, ASSIGNMENT, CALL,
		MODULE, FUNCTION, BLOCK, IF_STATEMENT, WHILE_STATEMENT, BINARY_OPERATION, RETURN
	};

	constexpr char* const NODE_TYPE_MNEMONIC[] = {
		"UNKNOWN", "CONSTANT", "ARGUMENTS", "SYMBOL", "DATA_TYPE", "ASSIGNMENT", "CALL",
		"MODULE", "FUNCTION", "BLOCK", "IF_STATEMENT", "WHILE_STATEMENT", "BINARY_OPERATION", "RETURN"
	};

	class VMNode {
	public:

		VMNode(Token token, VMNodeType type);
		~VMNode();

		VMNode* addChild(VMNode* node);
		bool removeChild(VMNode* node);
		void removeAll();

		VMNodeType getType();
		Token getToken();
		VMNode* getParent();
		VMNode* getChild(size_t index);
		size_t getChildCount();
		size_t getDepth();

		void print();

	private:
		Token token;
		vector<VMNode*> childs;
		VMNode* parent;
		VMNodeType type;

		void print(int tab);
	};

};