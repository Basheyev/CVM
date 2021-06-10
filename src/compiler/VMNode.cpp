/*============================================================================
*
*  Abstract Syntax Tree Node class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "compiler/VMNode.h"
#include <iostream>

using namespace vm;
using namespace std;

VMNode::VMNode(VMNode* parent = NULL, Token* token = NULL) {
	this->parent = parent;
	this->token = token;
}


VMNode::~VMNode() {
	size_t childCount = childs.size();
	for (size_t i = 0; i < childCount; i++) {
		delete childs[i];
		childs[i] = NULL;
	}
	childs.clear();
}


VMNode* VMNode::addChild(Token& token) {
	VMNode* node = new VMNode(this, &token);
	childs.push_back(node);
	return node;
}


bool VMNode::removeChild(VMNode* node) {
	for (auto entry = begin(childs); entry != end(childs); ++entry) {
		if (*entry==node) {
			childs.erase(entry);
			return true;
		}
	}
	return false;
}


Token* VMNode::getToken() {
	return token;
}


VMNode* VMNode::getParent() {
	return parent;
}

VMNode* VMNode::getChild(size_t index) {
	return childs.at(index);
}


size_t VMNode::getChildCount() {
	return childs.size();
}


void VMNode::print(int tab) {

	if (token != NULL) {
		for (int i = 0; i < tab; i++) cout << "  ";
		cout.write(token->text, token->length);
		cout << endl;
	}

	for (auto& node : childs) {
		node->print(tab + 1);
	}
}
