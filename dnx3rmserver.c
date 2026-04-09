#include <windows.h>
#include <sig.hpp>
#include <MinHook.h>
#include <cstdint>
#include <psapi.h>
#include <stdio.h>

#define CHECK(x,r) \
	if(x==NULL) return notify(r,__LINE__);

#define sfprintf(f,...) \
	if(f) {\
		fwprintf(f,__VA_ARGS__); \
		fflush(f); \
	}


FILE *fptr;

typedef void (*PushPathAFunc)(LPCSTR param);

extern "C" {
void PushPathA(LPCSTR param);
}

PushPathAFunc dummy0 = &PushPathA;
typedef int64_t (*functype)(LPCRITICAL_SECTION param);
functype ofunc = NULL;
int64_t myfunc(LPCRITICAL_SECTION param) {
	sfprintf(fptr,L"called\n");
	int64_t ret = 0;
	return (ret = ofunc(param)) == 1 ? ret : 1;
}

const char* const pt = "40 53 48 83 EC 20 ?? ?? ?? ?? ?? ?? ?? ?? 48 8B D9 74 17 4C 8D 05 46";


inline DWORD notify(int v,int l) {
	if(v != 1) {
		sfprintf(fptr,L"nox3 failed ! At %s:%d\nPattern is > %s\n",__FILE__,l,pt);
	}
	return v;
}

DWORD work() {
	HMODULE exe = GetModuleHandle(L"zwave_sdk_helper_x64.dll"); CHECK(exe,0)
	MODULEINFO exeinf = {0};
	LPMODULEINFO pexeinf = &exeinf;
	if(!GetModuleInformation(GetCurrentProcess(),exe,pexeinf,sizeof(exeinf))) return notify(0,__LINE__);
	const void* FP = Sig::find(exe, exeinf.SizeOfImage, pt);
	CHECK(FP,0)
	sfprintf(fptr,L"X3 Checker found at %p !\n",FP);
	if(MH_CreateHook((LPVOID)FP,(LPVOID)&myfunc,(LPVOID*)&ofunc) != MH_OK) return notify(0,__LINE__);
	if(MH_EnableHook((LPVOID)FP) != MH_OK) return notify(0,__LINE__);
	return 1;
}

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	LPCWSTR logname = L"_nox3.log";
	TCHAR exename[96] = {};
	TCHAR flogname[128] = {};
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH:
			GetModuleFileNameEx(GetCurrentProcess(),NULL,exename,96);
			wcsncat(flogname,exename,96);
			wcsncat(flogname,logname,wcslen(logname));
			fptr = _wfopen(flogname, L"w");
			sfprintf(fptr,L"!!Simple X3 ServerMode Hecker!!\nFor DragonNest-3618\nBy novice gamer EBK21\nem: eebssk1@godaftwithebk.pub\n");
			sfprintf(fptr,L"Start...\n");
			if (MH_Initialize() != MH_OK) { sfprintf(fptr,L"Error !\n"); fclose(fptr); return false; }
			if(!work()) { fclose(fptr); return false;}
			break;
		default:
			break;
	}
	return true;
}
