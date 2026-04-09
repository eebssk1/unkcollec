#include <windows.h>
static int size;

typedef void (*xat)(void);

extern "C"
void xaxaxaxb(void);

__attribute__((used)) xat xaa = &xaxaxaxb;

extern "C"
BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved) {
			size = GetCurrentDirectory(0, NULL);
			TCHAR path[size+2] = {};
switch( fdwReason ) {
                case DLL_PROCESS_ATTACH:
			if(GetCurrentDirectory(size+1, path) != 0) {
				SetDllDirectoryW(path);
			}
		default:
			break;
}
return true;
}
