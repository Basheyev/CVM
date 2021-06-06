/*============================================================================
*
*  Source Code lexer class implementation
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include "VMLexer.h"
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace vm;



VMLexer::VMLexer() {
	tokens = new vector<Token>();
	rowCounter = 0;
	rowPointer = NULL;
}


VMLexer::~VMLexer() {
	delete tokens;
}


void VMLexer::parseToTokens(const char* sourceCode) {

	TokenType isNumber = TokenType::UNKNOWN;
	bool insideString = false;                                         // inside string flag
	bool isReal = false;                                               // is real number flag
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
		length = cursor - start;                                       // measure token length
		
        // Diffirentiaite real numbers from member access operator '.'
		isNumber = identifyNumber(start, length - 1);                  // Try to get integer part of real number
		isReal = (value=='.' && isNumber==TokenType::CONST_INTEGER);   // Is current token is real number

		if ((blank || delimeter) && !insideString && !isReal) {        // if there is token separator                   
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
			// TODO warn about parsing error
		}
		cursor++;                                                      // increment cursor pointer
		value = *cursor;                                               // read next char
	}

	length = cursor - start;                                           // if there is a last token
	if (length > 0) pushToken(start, length);                          // push last token to vector

}


bool VMLexer::isBlank(char value) {
	return value == '\x20' || value == '\t' || value == '\n';
}


bool VMLexer::isDelimeter(char value) {
	char* cursor = DELIMETERS;
	while (*cursor != NULL) {
		if (*cursor == value) return true;
		cursor++;
	}
	return false;
}


TokenType VMLexer::getTokenType(char* text, size_t length) {
	char value = *text;
	if (length == 2) {
		if (strncmp(text, "==", 2) == 0) return TokenType::EQUAL;
		if (strncmp(text, "!=", 2) == 0) return TokenType::NOT_EQUAL;
		if (strncmp(text, ">=", 2) == 0) return TokenType::GR_EQUAL;
		if (strncmp(text, "<=", 2) == 0) return TokenType::LS_EQUAL;
		if (strncmp(text, "<<", 2) == 0) return TokenType::SHL;
		if (strncmp(text, ">>", 2) == 0) return TokenType::SHR;
		if (strncmp(text, "&&", 2) == 0) return TokenType::LOGIC_AND;
		if (strncmp(text, "||", 2) == 0) return TokenType::LOGIC_OR;
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
		case '*': return TokenType::MULTIPLY;
		case '/': return TokenType::DIVIDE;
		case '&': return TokenType::AND;
		case '|': return TokenType::OR;
		case '^': return TokenType::XOR;
		case '~': return TokenType::NOT;
		case '>': return TokenType::GREATER;
		case '<': return TokenType::LESS;
		case '!': return TokenType::LOGIC_NOT;
		case '.': return TokenType::MEMBER_ACCESS;
	}
	if (value == '\'' && length >= 3 && text[length - 1] == '\'') return TokenType::CONST_CHAR;
	if (value == '"' && length >= 2 && text[length - 1] == '"') return TokenType::CONST_STRING;
	if (isdigit(value)) return identifyNumber(text, length);
	if (isalpha(value)) return identifyKeyword(text, length);

	return TokenType::UNKNOWN;
}


TokenType VMLexer::identifyNumber(char* text, size_t length) {
	bool pointFound = false;
	char value;
	for (size_t i = 1; i < length; i++) {
		value = text[i];
		if (!isdigit(value)) {
			if (pointFound && value == '.') return TokenType::UNKNOWN;
			if (value == '.') pointFound = true; else return TokenType::UNKNOWN;
		}
	}
	return pointFound ? TokenType::CONST_REAL : TokenType::CONST_INTEGER;
}



TokenType VMLexer::identifyKeyword(char* text, size_t length) {
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

	// if it is not keyword, check if is it correct identifier
	for (size_t i = 1; i < length; i++) {
		if (!isalnum(text[i])) return TokenType::UNKNOWN;
	}

	return TokenType::IDENTIFIER;
}


TokenType VMLexer::pushToken(char* text, size_t length) {
	TokenType type = getTokenType(text, length);
	if (type != TokenType::UNKNOWN) {
		WORD tokenColumn = (WORD)(text - rowPointer);
		tokens->push_back({ type, text, (WORD)length, rowCounter, tokenColumn + 1 });
	}
	else {
		// TODO warn about unknown token we didnt pushed to vector
	}
	return type;
}


//-----------------------------------------------------------------------------------------------
// Parsed data getter methods
//-----------------------------------------------------------------------------------------------

Token VMLexer::getToken(size_t index) {
	if (index >= tokens->size()) {
		return { TokenType::NONE, "NONE", 0, 0, 0 };
	}
	return tokens->at(index);
}


WORD VMLexer::tokenToInt(Token& tkn) {
	if (tkn.type != TokenType::CONST_INTEGER) return 0;
	char buffer[32];
	strncpy_s(buffer, tkn.text, tkn.length);
	buffer[tkn.length] = 0;
	return atoi(buffer);
}


size_t VMLexer::getTokenCount() {
	return tokens->size();
}




void VMLexer::printToken(Token& tkn) {
	cout << "Line=" << tkn.row << " Col=" << tkn.col << "\t";
	cout.write(tkn.text, tkn.length);
	cout << "\t" << "length: " << tkn.length;
	cout << "\ttype: " << TOKEN_TYPE_MNEMONIC[(int)tkn.type] << endl;
}


void VMLexer::printAllTokens() {
	Token tkn;
	for (int i = 0; i < getTokenCount(); i++) {
		tkn = getToken(i);
		printToken(tkn);
	}
}