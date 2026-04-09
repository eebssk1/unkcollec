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

typedef void (*functype)(int64_t,int64_t,int64_t,int64_t);
functype onoop = NULL;
void noop(int64_t a,int64_t b,int64_t c,int64_t d) {
	sfprintf(fptr,L"called\n");
	return;
}

const char* const pt = "48 8B C4 55 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC 90 00 00 00 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 48 8B 05 9E";


inline DWORD notify(int v,int l) {
	if(v != 1) {
		sfprintf(fptr,L"nox3 failed ! At %s:%d\nPattern is > %s\n",__FILE__,l,pt);
	}
	return v;
}

DWORD work() {
	HMODULE exe = GetModuleHandle(NULL); CHECK(exe,0)
	MODULEINFO exeinf = {0};
	LPMODULEINFO pexeinf = &exeinf;
	if(!GetModuleInformation(GetCurrentProcess(),exe,pexeinf,sizeof(exeinf))) return notify(0,__LINE__);
	const void* FP = Sig::find(exe, exeinf.SizeOfImage, pt);
	CHECK(FP,0)
	sfprintf(fptr,L"X3 Init found at %p !\n",FP);
	if(MH_CreateHook((LPVOID)FP,(LPVOID)&noop,(LPVOID*)&onoop) != MH_OK) return notify(0,__LINE__);
	if(MH_EnableHook((LPVOID)FP) != MH_OK) return notify(0,__LINE__);
	return 1;
}

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH:
			fptr = _wfopen(L"nox3.log", L"w");
			sfprintf(fptr,L"!!Simple X3 ClientMode Hecker!!\nFor DragonNest-3618\nBy novice gamer EBK21\nem: eebssk1@godaftwithebk.pub\n");
			sfprintf(fptr,L"Start...\n");
			if (MH_Initialize() != MH_OK) { sfprintf(fptr,L"Error !\n"); fclose(fptr); return false; }
			if(!work()) { fclose(fptr); return false;}
			break;
		default:
			break;
	}
	return true;
}
