#include<stdio.h>
#include<Windows.h>

void EnumerateFileInDirectory(LPCWSTR FilePath) {
	WIN32_FIND_DATA FindFileData;
	HANDLE ListFile;
	DWORD rWitten;
	HANDLE hFile;
	wchar_t szFilePath[MAX_PATH];
	wchar_t szFullPath[MAX_PATH];
	DWORD Mnum2;

	lstrcpy(szFilePath,FilePath);
	lstrcat(szFilePath, L"\\*");


	hFile = CreateFile(
		L"systemroot.txt",
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	ListFile = FindFirstFile(szFilePath, &FindFileData);
	
	if (ListFile == INVALID_HANDLE_VALUE) {
		printf("错误：%d \n", GetLastError());
		
	}
	
	else {
		do {
		
			if (lstrcmp(FindFileData.cFileName, L".") == 0 || lstrcmp(FindFileData.cFileName, L"..") == 0) {
				continue;
			}
			wsprintf(szFullPath,L"%ls\\%ls", FilePath, FindFileData.cFileName);	
			Mnum2 = WideCharToMultiByte(CP_OEMCP, 0, szFullPath, -1, NULL, 0, NULL, NULL);
			char* d = new char[Mnum2];
			WideCharToMultiByte(CP_OEMCP, 0, szFullPath, -1, d, Mnum2, NULL, NULL);	
			WriteFile(hFile, d, strlen(d), &rWitten, NULL);
			SetFilePointer(hFile, 0, NULL, FILE_END);

		} while (FindNextFile(ListFile, &FindFileData));
	}

	CloseHandle(hFile);
}

int main() {
	EnumerateFileInDirectory(L"E:\\protect\\Project1");
}