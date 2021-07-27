/*============================================================================
*
*  Virtual Machine class implementation
*
*  Lightweight 32-bit stack virtual machine runtime.
*
*  (C) Bolat Basheyev 2021
*
============================================================================*/
#include <iostream>
#include <cstring>
#include "runtime/VirtualMachine.h"
#include "runtime/ExecutableImage.h"

using namespace std;
using namespace vm;

//-----------------------------------------------------------------------------
// Allocates virtual machine RAM in bytes
//-----------------------------------------------------------------------------
VirtualMachine::VirtualMachine(WORD memorySize) {
	maxAddress = memorySize / sizeof(WORD);
	memory = new WORD[maxAddress];
	memset(memory, 0, maxAddress);
}

//-----------------------------------------------------------------------------
// Releases RAM of virtual machine
//-----------------------------------------------------------------------------
VirtualMachine::~VirtualMachine() {
	delete[] memory;
}

//-----------------------------------------------------------------------------
// Loads executable image to virtual machine RAM
//-----------------------------------------------------------------------------
bool VirtualMachine::loadImage(void* image, WORD bytesCount) {
	if (bytesCount > maxAddress * sizeof(WORD)) return false;
	memcpy(memory, image, bytesCount);
	return true;
}

//----------------------------------------------------------------------------
// Starts execution from address [0x0000]
//----------------------------------------------------------------------------
void VirtualMachine::execute() {
	WORD a = 0;				    // temporary variables
	WORD b = 0;                 // temporary variables

	ip = 0;                     // Set Instruction pointer to 0
	sp = maxAddress;            // Set Stack pointer to highest address
	fp = sp;                    // Set Frame pointer to Stack pointer
	lp = sp - 1;                // Set Locals pointer to Stack pointer - 1

	fetch: 
	switch (memory[ip++]) {
		//------------------------------------------------------------------------
		// STACK OPERATIONS
		//------------------------------------------------------------------------
		case OP_CONST: 
			memory[--sp] = memory[ip++]; 
			goto fetch;
		case OP_PUSH:
			a = memory[ip++];
			memory[--sp] = memory[a];
			goto fetch;
		case OP_POP:  
			a = memory[ip++];
			memory[a] = memory[sp++]; 
			break;
		case OP_DUP:
			a = memory[sp];
			memory[--sp] = a;
			goto fetch;
		//------------------------------------------------------------------------
		// ARITHMETIC OPERATIONS
		//------------------------------------------------------------------------
		case OP_INC:
			memory[sp]++;
			goto fetch;
		case OP_DEC:
			memory[sp]--;
			goto fetch;
		case OP_ADD:  
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a + b;
			goto fetch;
		case OP_SUB:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a - b;
			goto fetch;
		case OP_MUL:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a * b;
			goto fetch;
		case OP_DIV:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a / b;
			goto fetch;
		//------------------------------------------------------------------------
		// BITWISE OPERATIONS
		//------------------------------------------------------------------------
		case OP_AND:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a & b;
			goto fetch;
		case OP_OR:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a | b;
			goto fetch;
		case OP_XOR:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a ^ b;
			goto fetch;
		case OP_NOT:
			a = memory[sp++];
			memory[--sp] = ~a;
			goto fetch;
		case OP_SHL:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a << b;
			goto fetch;
		case OP_SHR:
			b = memory[sp++];
			a = memory[sp++];
			memory[--sp] = a >> b;
			goto fetch;
		//------------------------------------------------------------------------
		// FLOW CONTROL OPERATIONS (Relative jumps depending on top of the stack)
		//------------------------------------------------------------------------
		case OP_JMP:
			ip += memory[ip];
			goto fetch;
		case OP_IFEQ:
			a = memory[sp++];
			if (a == 0) ip += memory[ip]; else ip++;
			goto fetch;
		case OP_IFNE:
			a = memory[sp++];
			if (a != 0) ip += memory[ip]; else ip++;
			goto fetch;
		case OP_IFGR:
			a = memory[sp++];
			if (a > 0) ip += memory[ip]; else ip++;
			goto fetch;
		case OP_IFGE:
			a = memory[sp++];
			if (a >= 0) ip += memory[ip]; else ip++;
			goto fetch;
		case OP_IFLS:
			a = memory[sp++];
			if (a < 0) ip += memory[ip]; else ip++;
			goto fetch;
		case OP_IFLE:
			a = memory[sp++];
			if (a <= 0) ip += memory[ip]; else ip++;
			goto fetch;
		//------------------------------------------------------------------------
		// PROCEDURE CALL OPERATIONS
		//------------------------------------------------------------------------
		case OP_CALL:
			a = memory[ip++];      // get call address and increment address
			b = memory[ip++];      // get arguments count (argc)
			b = sp + b;            // calculate new frame pointer
			memory[--sp] = ip;     // push return address to the stack
			memory[--sp] = fp;     // push old Frame pointer to stack
			memory[--sp] = lp;     // push old Local variables pointer to stack
			fp = b;                // set Frame pointer to arguments pointer
			lp = sp - 1;           // set Local variables pointer after top of a stack
			ip = a;                // jump to call address
			goto fetch;
		case OP_RET:
			a = memory[sp++];      // read function return value on top of a stack
			b = lp;                // save Local variables pointer
			sp = fp;               // set stack pointer to Frame pointer (drop locals)
			lp = memory[b + 1];    // restore old Local variables pointer
			fp = memory[b + 2];    // restore old Frame pointer
			ip = memory[b + 3];    // set IP to return address
			memory[--sp] = a;      // save return value on top of a stack
			goto fetch;
		case OP_SYSCALL:
			a = memory[ip++];      // read system call index from top of the stack
			sysCall(a);         // make system call by index
			goto fetch;
		case OP_HALT: 
			printState();
			return;
		//------------------------------------------------------------------------
		// LOCAL VARIABLES AND CALL ARGUMENTS OPERATIONS
		//------------------------------------------------------------------------
		case OP_LOAD:
			a = memory[ip++];         // read local variable index
			b = lp - a;               // calculate local variable address
			memory[--sp] = memory[b]; // push local variable to stack
			goto fetch;
		case OP_STORE:
			a = memory[ip++];         // read local variable index
			b = lp - a;               // calculate local variable address
			memory[b] = memory[sp++]; // pop top of stack to local variable
			goto fetch;
		case OP_ARG:
			a = memory[ip++];         // read parameter index
			b = fp - a - 1;           // calculate parameter address
			memory[--sp] = memory[b]; // push parameter to stack
			goto fetch;
		case OP_DROP:                 // pop and drop value from stack
			sp++;
			goto fetch;
		default:
			cout << "Runtime error - unknown opcode at [" << ip << "]" << endl;
			printState();
			return;
	}

	goto fetch;

}

//----------------------------------------------------------------------------
// SYSCALL implementation
//----------------------------------------------------------------------------
void VirtualMachine::sysCall(WORD n) {
	WORD ptr, a;
	switch (n) {
	case 0x20:  // print C style string
		ptr = memory[sp++];
		cout << ((char*)&memory[ptr]);
		return;
	case 0x21:  // print int from TOS
		a = memory[sp++];
		cout << a << endl;
		return;
	}
}

//----------------------------------------------------------------------------
// Prints IP, SP, FP, LP and STACK to standard out
//----------------------------------------------------------------------------
void VirtualMachine::printState() {
	cout << "VM:";
	cout << " IP=" << ip;
	cout << " FP=" << fp;
	cout << " LP=" << lp;
	cout << " SP=" << sp;
	cout << " STACK=[";
	for (WORD i = maxAddress - 1; i >= sp; i--) {
		cout << memory[i];
		if (i > sp) cout << ",";
	}
	cout << "] -> TOP" << endl;
}
