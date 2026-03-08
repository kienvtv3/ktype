#include "pch.h"
#include "dll_main.h"

// Standard COM DLL exports, forwarded to ATL module
STDAPI DllCanUnloadNow() {
    return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}
