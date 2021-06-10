/*============================================================================
*
*  Abstract Syntax Tree Node class header
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/

#pragma once

#include "VMLexer.h"
#include <vector>

namespace vm {

	class VMNode {
	public:

		VMNode(VMNode* parent, Token* token);
		~VMNode();

		VMNode* addChild(Token& token);
		bool removeChild(VMNode* node);

		Token* getToken();
		VMNode* getParent();
		VMNode* getChild(size_t index);
		size_t getChildCount();

		void print(int tab = 0);

	private:
		VMNode* parent;
		vector<VMNode*> childs;
		Token* token;
	};

};