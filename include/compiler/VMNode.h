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

	class VMNode {
	public:

		VMNode(Token token);
		~VMNode();

		VMNode* addChild(VMNode* node);
		bool removeChild(VMNode* node);
		void removeAll();

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

		void print(int tab);
	};

};