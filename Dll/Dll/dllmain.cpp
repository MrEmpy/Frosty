#include "pch.h"
#include "requirements.h"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {
	if (!DetourIsHelperProcess()) {
		return 1;
	}

	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		StartHook();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		StopHook();
		break;
	}

	return 0;
}