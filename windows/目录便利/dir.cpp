#include<stdio.h>
#include<windows.h>

DWORD EnumerateFileInDirectory(LPCWSTR szPath){
    WIN32_FIND_DATA FindFileData;
    HANDLE hListFile;
    wchar_t szFilePath[MAX_PATH];
    wchar_t szFullPath[MAX_PATH]; 

    lstrcpy(szFilePath, szPath);
    lstrcat(szFilePath, L"\\*");

    hListFile = FindFirstFile(szFilePath, &FindFileData);
    if(hListFile == INVALID_HANDLE_VALUE){
        printf("错误：%d \n", GetLastError());
        return 1;
    }else {
        do {
            if (lstrcmp(FindFileData.cFileName, L".") == 0 || lstrcmp(FindFileData.cFileName, L"..") == 0){
                continue;
            }


            wsprintf(szFullPath, L"%ls\\%ls", szPath, FindFileData.cFileName);

            printf("\n%ls\t", szFullPath);

            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { 

                EnumerateFileInDirectory(szFullPath);
            }
            
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {

            }



        } while (FindNextFile(hListFile, &FindFileData));
    }
    printf("\n");
    return 0;
}



int main() {

    EnumerateFileInDirectory(L"C:\\Users\\dell\\source\\repos\\msgboxtest");
    system("pause");
    return 0;
}