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


VMNode::VMNode(Token token, VMNodeType type) {
	this->parent = NULL;
	this->token = token;
	this->type = type;
}


VMNode::~VMNode() {
	removeAll();
}


VMNode* VMNode::addChild(VMNode* node) {
	if (node == NULL) return NULL;
	node->parent = this;
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


void VMNode::removeAll() {
	size_t childCount = childs.size();
	for (size_t i = 0; i < childCount; i++) {
		delete childs[i];
		childs[i] = NULL;
	}
	childs.clear();
}


Token VMNode::getToken() {
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


size_t VMNode::getDepth() {
	size_t depth = 0;
	VMNode* node = getParent();
	while (node != NULL) {
		depth++;
		node = node->getParent();
	}
	return depth;
}


void VMNode::print() {
	print(0);
}


void VMNode::print(int tab) {
	for (int i = 0; i < tab; i++) if (i < tab - 1) cout << "| "; else cout << "|-";
	cout << "'";
	cout.write(token.text, token.length);
	cout << "'" << "(" << NODE_TYPE_MNEMONIC[(unsigned int) type] << ")" << endl;
	for (auto& node : childs) node->print(tab + 1);
}
