#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>
#include "../config.h"

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

VOID ExitRKServiceProcess(HANDLE pipe) {
    DWORD dwWritten;
    char buffer[4];

    if (pipe != INVALID_HANDLE_VALUE)
    {
        snprintf(buffer, sizeof(RK_UNINSTALL), "0x%x", RK_UNINSTALL);

        WriteFile(pipe,
            buffer,
            sizeof(buffer),
            &dwWritten,
            NULL);

        CloseHandle(pipe);
    }
}

BOOL DeleteService() {
    SC_HANDLE schSCManager, schService;
    SERVICE_STATUS serviceStatus;

    schSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        printf("[-] Error opening Service Control Manager: %lu\n", GetLastError());
        return 1;
    }

    schService = OpenServiceW(schSCManager, RK_SERVICE_NAME, SERVICE_STOP | DELETE);
    if (schService == NULL) {
        printf("[-] Error opening the service: %lu\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return 1;
    }

    if (ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus)) {
        printf("[+] Service stopped successfully\n");
    }
    else {
        puts("[-] It was not possible to stop the service, perhaps it is already stopped. Continuing...");
    }

    if (DeleteService(schService)) {
        puts("[+] Service uninstalled successfully!");
    }
    else {
        printf("[-] Error deleting service: %lu\n", GetLastError());
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return 0;
}

DWORD GetPID(LPCWSTR pn)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pE;
        pE.dwSize = sizeof(pE);

        if (Process32First(hSnap, &pE))
        {
            if (!pE.th32ProcessID)
                Process32Next(hSnap, &pE);
            do
            {
                if (!lstrcmpiW((LPCWSTR)pE.szExeFile, pn))
                {
                    procId = pE.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pE));
        }
    }
    CloseHandle(hSnap);
    return (procId);
}

VOID RestoreExplorer() {
    DWORD pid = GetPID(L"explorer.exe");

    HANDLE process = OpenProcess(PROCESS_TERMINATE, 0, pid);
    if (process != NULL) {
        TerminateProcess(process, NULL);
        CloseHandle(process);
    }

    Sleep(1000);
    ShellExecuteA(NULL, "open", "explorer.exe", NULL, NULL, SW_SHOWNORMAL);
}

VOID DeleteRKFiles() {
    LPCWSTR rkFiles[] = {RK_DLL_PATH , RK_SERVICE_PATH};

    for (int i = 0; i < sizeof(rkFiles) / sizeof(rkFiles[0]); i++) {
        if (DeleteFileW(rkFiles[i])) {
            printf("[+] File %ls deleted!\n", rkFiles[i]);
        }
        else {
            DWORD error = GetLastError();
            printf("[-] Unable to delete file %ls [%d]\n", rkFiles[i], error);
        }
    }
}

int main()
{
    if (IsElevated() == 0) {
        puts("[-] You need administrative privilege to uninstall the rootkit");
        return 1;
    }

    HANDLE pipe = CreateFileW(RK_PIPE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    ExitRKServiceProcess(pipe);

    RestoreExplorer();

    DeleteService();

    DeleteRKFiles();

    puts("[+] Frosty Rootkit uninstalled successfully!");
}