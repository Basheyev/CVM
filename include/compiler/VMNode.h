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

		VMNode();
		VMNode(Token token);
		VMNode(VMNode* parent, Token token);
		~VMNode();

		VMNode* addChild(VMNode* node);
		VMNode* addChild(Token token);
		bool removeChild(VMNode* node);
		void removeAll();

		Token getToken();
		VMNode* getParent();
		VMNode* getChild(size_t index);
		size_t getChildCount();
		size_t getDepth();

		void print();

	private:
		VMNode* parent;
		vector<VMNode*> childs;
		Token token;

		void print(int tab);
	};

};