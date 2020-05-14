#include<Windows.h>
#include<stdio.h>

using namespace std;

int main(int argc,char * argv[]) {
	HANDLE ProcessHandle;
	LPVOID remotebuffer;
	BOOL write;

	wchar_t dllpath[] = TEXT("C:\\users\\root\\desktop\\inject.dll");

	if (argc < 2) {
		printf("Useage inject.exe Pid;\n");
		printf("such as inject.exe 258\n");
		exit(0);
	}

	printf("Injecting DLL to PID: %i\n", atoi(argv[1]));
	ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DWORD(atoi(argv[1])));
	if (ProcessHandle == NULL) {
		printf("OpenProcess Fail !!!");
		exit(0);
	}
	else
	{
		printf("OpenProcess %i successful !!!\n",atoi(argv[1]));
	}

	remotebuffer = VirtualAllocEx(ProcessHandle, NULL, sizeof dllpath, MEM_COMMIT, PAGE_READWRITE);
	write = WriteProcessMemory(ProcessHandle, remotebuffer, (LPVOID)dllpath, sizeof dllpath, NULL);
	
	if (write == 0) {
		printf("WriteProcessMemory Fail %i!!!",GetLastError());
		exit(0);
	}
	else
	{
		printf("WriteProcessMemory  successful !!!\n");
	}

	PTHREAD_START_ROUTINE threatStartRoutineAddress = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	CreateRemoteThread(ProcessHandle, NULL, 0, threatStartRoutineAddress, remotebuffer, 0, NULL);
	CloseHandle(ProcessHandle);


	return 0;
}