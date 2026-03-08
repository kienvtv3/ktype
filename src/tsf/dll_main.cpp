#include "pch.h"
#include "dll_main.h"
#include "Globals.h"

CKTypeModule _AtlModule;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        if (!hInstance) return FALSE;
        KType::Globals::DllInstance = hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return _AtlModule.DllMain(dwReason, lpReserved);
}
