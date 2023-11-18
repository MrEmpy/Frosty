#pragma once
#include "requirements.h"

#define STATUS_NO_MORE_FILES 0x80000006
#define STATUS_NO_MORE_ENTRIES 0x8000001A
#define AMSI_IS_SAFE 0x80070057

typedef NTSTATUS(NTAPI* typedefNtQueryDirectoryFile)(
	HANDLE                 FileHandle,
	HANDLE                 Event,
	PIO_APC_ROUTINE        ApcRoutine,
	PVOID                  ApcContext,
	PIO_STATUS_BLOCK       IoStatusBlock,
	PVOID                  FileInformation,
	ULONG                  Length,
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN                ReturnSingleEntry,
	PUNICODE_STRING        FileName,
	BOOLEAN                RestartScan
);

typedef NTSTATUS(NTAPI* typedefNtQueryDirectoryFileEx)(
	HANDLE                 FileHandle,
	HANDLE                 Event,
	PIO_APC_ROUTINE        ApcRoutine,
	PVOID                  ApcContext,
	PIO_STATUS_BLOCK       IoStatusBlock,
	PVOID                  FileInformation,
	ULONG                  Length,
	FILE_INFORMATION_CLASS FileInformationClass,
	ULONG                  QueryFlags,
	PUNICODE_STRING        FileName
);

typedef NTSTATUS(NTAPI* typedefNtQuerySystemInformation)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID                    SystemInformation,
	ULONG                    SystemInformationLength,
	PULONG                   ReturnLength
);

typedef BOOL(WINAPI* typedefEnumServiceGroupW)(
	SC_HANDLE serviceManager,
	DWORD serviceType,
	DWORD serviceState,
	LPBYTE services,
	DWORD servicesLength,
	LPDWORD bytesNeeded,
	LPDWORD servicesReturned,
	LPDWORD resumeHandle,
	LPVOID reserved
);

typedef BOOL(WINAPI* typedefEnumServicesStatusExW)(
	SC_HANDLE serviceManager,
	SC_ENUM_TYPE infoLevel,
	DWORD serviceType,
	DWORD serviceState,
	LPBYTE services,
	DWORD servicesLength,
	LPDWORD bytesNeeded,
	LPDWORD servicesReturned,
	LPDWORD resumeHandle,
	LPCWSTR groupName
);

typedef HRESULT(WINAPI* typedefAmsiScanBuffer)(
	HAMSICONTEXT context,
	void* buffer,
	ULONG length,
	const WCHAR* name,
	HAMSISESSION session,
	AMSI_RESULT* result
);