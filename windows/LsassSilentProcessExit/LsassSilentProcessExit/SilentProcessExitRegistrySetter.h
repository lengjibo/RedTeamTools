#pragma once
#include <Windows.h>
#include <string>


#define IFEO_REG_KEY "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\"
#define SILENT_PROCESS_EXIT_REG_KEY "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SilentProcessExit\\"
#define LOCAL_DUMP 0x2
#define FLG_MONITOR_SILENT_PROCESS_EXIT 0x200
#define DUMP_FOLDER "C:\\temp"
#define MiniDumpWithFullMemory 0x2

class SilentProcessExitRegistrySetter
{
public:
	SilentProcessExitRegistrySetter(std::string processName);
	~SilentProcessExitRegistrySetter();

	BOOL isValid();

private:
	BOOL m_isValid;
	HKEY m_hIFEORegKey;
	HKEY m_hSPERegKey;
};