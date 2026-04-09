#include <windows.h>
#include <sig.hpp>
#include <MinHook.h>
#include <cstdint>
#include <psapi.h>
#include <stdio.h>
#include <string>

#define rt(a,b) \
        {MessageBox(NULL,b,NULL,0);a;}


typedef HRESULT (*LPCREATEBANDIVIDEO2)(uint32_t sdk_version, void** pp, void* reader, void *sound);

extern "C" {
HRESULT CreateBandiVideo2(uint32_t sdk_version, void** pp, void* reader, void *sound);
}

__attribute__((__used__)) static LPCREATEBANDIVIDEO2 dummy0 = &CreateBandiVideo2;

typedef __fastcall int32_t (*funcdel)(int32_t param);

funcdel ofunc = NULL;

__fastcall int32_t myfunc(int32_t param) {
	SetThreadPriority(GetCurrentThread(),-1);
	return ofunc(param);
}

const char* const pt = "55 8B EC 83 E4 F8 83 EC 08";

int work() {
	HMODULE dll = GetModuleHandle("bdvid32.dll");
	if(NULL == dll) rt(return 0,"a")
	MODULEINFO dllinf = {};
	LPMODULEINFO pdllinf = &dllinf;
	if(!GetModuleInformation(GetCurrentProcess(),dll,pdllinf,sizeof(dllinf))) rt(return 0,"b")
	const void* FP = Sig::find(dll, dllinf.SizeOfImage, pt);
	if(NULL == FP) rt(return 0,"-11");
	if(MH_CreateHook((LPVOID)FP,(LPVOID)&myfunc,(LPVOID*)&ofunc) != MH_OK) return 0;
	if(MH_EnableHook((LPVOID)FP) != MH_OK) return 0;
	return 1;
}

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH:
			if (MH_Initialize() != MH_OK) rt(return false, "nnn")
			return work() != 0 ? true : false;
			break;
		default:
			break;
	}
	return true;
}
