/*============================================================================
*
*  Virtual Machine Compiler Abstract Syntax Tree Node implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#pragma once

#include "compiler/SourceParser.h"

#include <iostream>

using namespace vm;
using namespace std;


TreeNode::TreeNode(Token token, TreeNodeType type, SymbolTable* scope) {
	this->parent = NULL;
	this->token = token;
	this->type = type;
	this->symbols = scope;
}


TreeNode::~TreeNode() {
	removeAll();
}


TreeNode* TreeNode::addChild(TreeNode* node) {
	if (node == NULL) return NULL;
	node->parent = this;
	childs.push_back(node);
	return node;
}


bool TreeNode::removeChild(TreeNode* node) {
	for (auto entry = begin(childs); entry != end(childs); ++entry) {
		if (*entry == node) {
			childs.erase(entry);
			return true;
		}
	}
	return false;
}


void TreeNode::removeAll() {
	size_t childCount = childs.size();
	for (size_t i = 0; i < childCount; i++) {
		delete childs[i];
		childs[i] = NULL;
	}
	childs.clear();
}


TreeNodeType TreeNode::getType() {
	return type;
}

Token TreeNode::getToken() {
	return token;
}


TreeNode* TreeNode::getParent() {
	return parent;
}

TreeNode* TreeNode::getChild(size_t index) {
	if (index >= childs.size()) return NULL;
	return childs.at(index);
}


size_t TreeNode::getChildCount() {
	return childs.size();
}


size_t TreeNode::getDepth() {
	size_t depth = 0;
	TreeNode* node = getParent();
	while (node != NULL) {
		depth++;
		node = node->getParent();
	}
	return depth;
}


void TreeNode::print() {
	cout << "-----------------------------------------------------" << endl;
	cout << "Parsed abstract syntax tree" << endl;
	cout << "-----------------------------------------------------" << endl;
	print(0);
}


void TreeNode::print(int tab) {
	for (int i = 0; i < tab; i++) if (i < tab - 1) cout << "| "; else cout << "|-";
	cout << "'";
	cout.write(token.text, token.length);
	cout << "'" << "(" << TREE_NODE_TYPE_MNEMONIC[(unsigned int)type] << ")";
	cout << " " << getSymbolTable()->getName() << endl;
	//if (symbols != NULL) symbols->printSymbols();
	for (auto& node : childs) node->print(tab + 1);
}
