#include <windows.h>
#include <MinHook.h>
#include <urlmon.h>
#include <cstdint>
#include <psapi.h>
#include <stdio.h>

FILE *fptr;

typedef HRESULT WINAPI (*funct)(LPUNKNOWN,LPCTSTR,LPCTSTR,DWORD,LPBINDSTATUSCALLBACK);

HMODULE hModule = NULL;

funct dummy0 = URLDownloadToFileA;

funct ofunc = NULL;
funct ifunc = NULL;
HRESULT WINAPI myfunc(LPUNKNOWN a,LPCTSTR b,LPCTSTR c,DWORD d,LPBINDSTATUSCALLBACK e) {
	fprintf(fptr,"nulled\n");
	fflush(fptr);
	return INET_E_DOWNLOAD_FAILURE;
}


int work() {
	ifunc = (funct)GetProcAddress(hModule, "URLDownloadToFileA");
	if(MH_CreateHook((LPVOID)ifunc,(LPVOID)&myfunc,(LPVOID*)&ofunc) != MH_OK) {return 1;}
	if(MH_EnableHook((LPVOID)ifunc) != MH_OK) {return 2;}
	return 0;
}

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	int ret = 0;
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH:
			fptr = fopen("ndl.log", "w");
			fprintf(fptr,"init\n");
			fflush(fptr);
			hModule = GetModuleHandleA("urlmon.dll");
			if(hModule == NULL) { fprintf(fptr,"urlmon not loaded!\n");fclose(fptr);return false;}
			if (MH_Initialize() != MH_OK) { fprintf(fptr,"Failed init ndl!\n");fclose(fptr);return false;}
			if((ret = work())){fprintf(fptr,ret == 1 ? "Failed hook create!\n" : "hook Failed !\n");fclose(fptr);return false;}
			break;
		default:
			break;
	}
	return true;
}
