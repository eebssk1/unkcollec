#include <windows.h>
#include <sig.hpp>
#include <MinHook.h>
#include <cstdint>
#include <psapi.h>
#include <stdio.h>
#include <iphlpapi.h>


typedef WINAPI DWORD (*GetIpAddrTableFunc)(PMIB_IPADDRTABLE, PULONG, BOOL);

GetIpAddrTableFunc dummy0 = &GetIpAddrTable;

GetIpAddrTableFunc ofunc = NULL;

WINAPI DWORD myfunc(PMIB_IPADDRTABLE pIpAddrTable, PULONG pdwSize, BOOL bOrder) {
	MIB_IPADDRTABLE tb = { .dwNumEntries = 1, .table = {{.dwAddr = 0, .dwIndex = 1, .dwMask = 0, .dwBCastAddr = 0, .dwReasmSize = 0, .wType = 4 }} };
	uint32_t size = sizeof(tb);
	if(size > *pdwSize) { *pdwSize = size; return ERROR_INSUFFICIENT_BUFFER;}
	memcpy(pIpAddrTable, &tb, size);
	return NO_ERROR;
}

DWORD work() {
	HMODULE dll = GetModuleHandle("iphlpapi.dll");
	if(!dll) return 0;
	MODULEINFO dllinf = {0};
	LPMODULEINFO pdllinf = &dllinf;
	if(!GetModuleInformation(GetCurrentProcess(),dll,pdllinf,sizeof(dllinf))) return 0;
	const LPVOID FP = (LPVOID)GetProcAddress(dll,"GetIpAddrTable");
	if(!FP) return 0;
	if(MH_CreateHook((LPVOID)FP,(LPVOID)&myfunc,(LPVOID*)&ofunc) != MH_OK) return 0;
	if(MH_EnableHook((LPVOID)FP) != MH_OK) return 0;
	return 1;
}

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH:
			if (MH_Initialize() != MH_OK) return false;
			if(work() != 1) return false;
			break;
		default:
			break;
	}
	return true;
}
