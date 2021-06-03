/*============================================================================
*
*  Virtual Machine Parser class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "VMParser.h"
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace vm;



VMParser::VMParser() {
	tokens = new vector<Token>();
	rowCounter = 0;
	rowPointer = NULL;
}


VMParser::~VMParser() {
	delete tokens;
}


void VMParser::parseToTokens(const char* sourceCode) {

	bool insideString = false;                                         // inside string flag
	bool insideNumber = false;                                         // inside number flag
	size_t length;                                                     // token length variable

	char nextChar;                                                     // next char variable
	bool blank, delimeter;                                             // blank & delimeter char flags

	tokens->clear();                                                   // clear tokens vector
	rowCounter = 1;                                                    // reset current row counter
	rowPointer = (char*)sourceCode;                                    // set current row pointer to beginning

	char* cursor = (char*)sourceCode;                                  // set cursor to source beginning 
	char* start = cursor;                                              // start new token from cursor
	char value = *cursor;                                              // read first char from cursor

	while (value != NULL) {                                            // while not end of string
		blank = isBlank(value);                                        // is blank char found?
		delimeter = isDelimeter(value);                                // is delimeter found?
		if ((blank || delimeter) && !insideString) {                   // if there is token separator                     
			length = cursor - start;                                   // measure token length
			if (length > 0) pushToken(start, length);                  // if length > 0 push token to vector
			if (value == '\n') {                                       // if '\n' found 
				rowCounter++;                                          // increment row counter
				rowPointer = cursor + 1;                               // set row beginning pointer
			}
			nextChar = *(cursor + 1);                                  // get next char after cursor
			if (!blank && isDelimeter(nextChar)) {                     // if next char is also delimeter
				if (pushToken(cursor, 2) == TokenType::UNKNOWN)        // try to push double char delimeter token
					pushToken(cursor, 1);                              // if not pushed - its single char delimeter
				else cursor++;                                         // if double delimeter, increment cursor
			} else pushToken(cursor, 1);                               // else push single char delimeter
			start = cursor + 1;                                        // calculate next token start pointer
		}
		else if (value == '"') insideString = !insideString;           // if '"' char - flip insideString flag 
		else if (insideString && value == '\n') {                      // if '\n' found inside string
			// TODO parsing error found
		}
		cursor++;                                                      // increment cursor pointer
		value = *cursor;                                               // read next char
	}

	length = cursor - start;                                           // if there is a last token
	if (length > 0) pushToken(start, length);                          // push last token to vector

}


bool VMParser::isBlank(char value) {
	return value == '\x20' || value == '\t' || value == '\n';
}


bool VMParser::isDelimeter(char value) {
	char* cursor = DELIMETERS;
	while (*cursor != NULL) {
		if (*cursor == value) return true;
		cursor++;
	}
	return false;
}


TokenType VMParser::pushToken(char* text, size_t length) {
	TokenType type = getTokenType(text, length);
	if (type != TokenType::UNKNOWN) {
		WORD tokenColumn = (WORD) (text - rowPointer);
		tokens->push_back({ type, text, (WORD) length, rowCounter, tokenColumn + 1});
	}
	else {
		// TODO warn about unknown token
	}
	return type;
}


TokenType VMParser::getTokenType(char* text, size_t length) {
	char value = *text;
	if (length == 2) {
		if (strncmp(text, "==", 2) == 0) return TokenType::EQUAL;
		if (strncmp(text, "!=", 2) == 0) return TokenType::NOT_EQUAL;
		if (strncmp(text, ">=", 2) == 0) return TokenType::GR_EQUAL;
		if (strncmp(text, "<=", 2) == 0) return TokenType::LS_EQUAL;
		if (strncmp(text, "<<", 2) == 0) return TokenType::SHL;
		if (strncmp(text, ">>", 2) == 0) return TokenType::SHR;
		if (strncmp(text, "&&", 2) == 0) return TokenType::L_AND;
		if (strncmp(text, "||", 2) == 0) return TokenType::L_OR;
	}
	else if (length == 1) switch (value) {
		case ';': return TokenType::EOS;
		case ',': return TokenType::COMMA;
		case '{': return TokenType::OP_BRACES;
		case '}': return TokenType::CL_BRACES;
		case '[': return TokenType::OP_BRACKETS;
		case ']': return TokenType::CL_BRACKETS;
		case '(': return TokenType::OP_PARENTHESES;
		case ')': return TokenType::CL_PARENTHESES;
		case '=': return TokenType::ASSIGN;
		case '+': return TokenType::PLUS;
		case '-': return TokenType::MINUS;
		case '*': return TokenType::MUL;
		case '/': return TokenType::DIV;
		case '&': return TokenType::AND;
		case '|': return TokenType::OR;
		case '^': return TokenType::XOR;
		case '~': return TokenType::NOT;
		case '>': return TokenType::GREATER;
		case '<': return TokenType::LESS;
		case '!': return TokenType::L_NOT;
		//case '.': return TokenType::MEMBER_ACCESS;
	}
	if (value == '\'' && length == 3) return TokenType::CONST_CHAR;
	if (value == '"' && length >= 3 && text[length - 1] == '"') return TokenType::CONST_STRING;
	if (isdigit(value)) return identifyNumber(text, length);
	if (isalpha(value)) return identifyKeyword(text, length);

	return TokenType::UNKNOWN;
}


TokenType VMParser::identifyNumber(char* text, size_t length) {
	bool pointFound = false;
	char value;
	for (size_t i = 1; i < length; i++) {
		value = text[i];
		if (!isdigit(value)) {
			if (pointFound && value == '.') return TokenType::UNKNOWN;
			if (value == '.') pointFound = true; else return TokenType::UNKNOWN;
		}
	}
	return pointFound ? TokenType::CONST_FLOAT : TokenType::CONST_INT;
}



TokenType VMParser::identifyKeyword(char* text, size_t length) {
	if (length == 4 && strncmp(text, "byte", 4) == 0) return TokenType::BYTE;
	if (length == 5 && strncmp(text, "short", 5) == 0) return TokenType::SHORT;
	if (length == 3 && strncmp(text, "int", 3) == 0) return TokenType::INT;
	if (length == 4 && strncmp(text, "long", 4) == 0) return TokenType::LONG;
	if (length == 4 && strncmp(text, "char", 4) == 0) return TokenType::CHAR;
	if (length == 5 && strncmp(text, "float", 5) == 0) return TokenType::FLOAT;
	if (length == 6 && strncmp(text, "double", 6) == 0) return TokenType::DOUBLE;
	if (length == 6 && strncmp(text, "string", 6) == 0) return TokenType::STRING;
	if (length == 2 && strncmp(text, "if", 2) == 0) return TokenType::IF;
	if (length == 4 && strncmp(text, "else", 4) == 0) return TokenType::ELSE;
	if (length == 5 && strncmp(text, "while", 5) == 0) return TokenType::WHILE;
	if (length == 6 && strncmp(text, "return", 6) == 0) return TokenType::RETURN;
	if (length == 1) return TokenType::IDENTIFIER;

	// if it is not keyword, check if it is correct identifier
	for (size_t i = 1; i < length; i++) {
		if (!isalnum(text[i])) return TokenType::UNKNOWN;
	}

	return TokenType::IDENTIFIER;
}


//-----------------------------------------------------------------------------------------------
// Parsed data getter methods
//-----------------------------------------------------------------------------------------------

Token VMParser::getToken(size_t index) {
	if (index >= tokens->size()) {
		return { TokenType::NONE, "NONE", 0, 0, 0 };
	}
	return tokens->at(index);
}

size_t VMParser::getTokenCount() {
	return tokens->size();
}

void VMParser::printToken(Token& tkn) {
	cout << "Token: ";
	cout.write(tkn.text, tkn.length);
	cout << "\tLength: " << (uint32_t)tkn.length;
	cout << "\tType: " << (int)tkn.type;
}


void VMParser::printAllTokens() {
	Token tkn;
	for (int i = 0; i < getTokenCount(); i++) {
		tkn = getToken(i);
		printf("Token: %.*s", (uint32_t)tkn.length, tkn.text);
		printf("\tlength=%d", (uint32_t)tkn.length);
		printf("\ttype=%s", TOKEN_TYPE_MNEMONIC[(int)tkn.type]);
		printf("\tLine=%d", tkn.row);
		printf("\tCol=%d\n", tkn.col);
	}
}