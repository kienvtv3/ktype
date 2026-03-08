#include "pch.h"
#include "globals.h"
#include "resource.h"

namespace KType {

// Convert CLSID to registry string "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
static std::wstring ClsidToString(REFCLSID clsid) {
    wchar_t buf[64];
    StringFromGUID2(clsid, buf, 64);
    return buf;
}

// Register COM server CLSID -> DLL path in the registry
static HRESULT RegisterCOMServer() {
    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(Globals::DllInstance, dllPath, MAX_PATH);

    std::wstring clsidStr = ClsidToString(Globals::CLSID_TextService);
    std::wstring keyPath = L"CLSID\\" + clsidStr;

    HKEY hKey;
    LONG result = RegCreateKeyExW(HKEY_CLASSES_ROOT, keyPath.c_str(),
        0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    if (result != ERROR_SUCCESS) return HRESULT_FROM_WIN32(result);

    // Default value = description
    RegSetValueExW(hKey, nullptr, 0, REG_SZ,
        (const BYTE*)Globals::TextServiceDescription,
        (DWORD)(wcslen(Globals::TextServiceDescription) + 1) * sizeof(wchar_t));
    RegCloseKey(hKey);

    // InprocServer32 subkey
    std::wstring inprocPath = keyPath + L"\\InprocServer32";
    result = RegCreateKeyExW(HKEY_CLASSES_ROOT, inprocPath.c_str(),
        0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    if (result != ERROR_SUCCESS) return HRESULT_FROM_WIN32(result);

    // Default value = DLL path
    RegSetValueExW(hKey, nullptr, 0, REG_SZ,
        (const BYTE*)dllPath, (DWORD)(wcslen(dllPath) + 1) * sizeof(wchar_t));

    // ThreadingModel = Apartment (required for TSF)
    const wchar_t* threadModel = L"Apartment";
    RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ,
        (const BYTE*)threadModel, (DWORD)(wcslen(threadModel) + 1) * sizeof(wchar_t));

    RegCloseKey(hKey);
    return S_OK;
}

static void UnregisterCOMServer() {
    std::wstring clsidStr = ClsidToString(Globals::CLSID_TextService);
    std::wstring keyPath = L"CLSID\\" + clsidStr;

    // Delete InprocServer32 subkey first, then the CLSID key
    std::wstring inprocPath = keyPath + L"\\InprocServer32";
    RegDeleteKeyW(HKEY_CLASSES_ROOT, inprocPath.c_str());
    RegDeleteKeyW(HKEY_CLASSES_ROOT, keyPath.c_str());
}

static HRESULT RegisterProfiles() {
    ATL::CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr,
        CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&profileMgr);
    if (FAILED(hr)) return hr;

    // Get DLL path
    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(Globals::DllInstance, dllPath, MAX_PATH);

    // Use US English keyboard layout underneath KType
    // Without this, Windows uses the default Vietnamese keyboard layout which
    // maps number keys to diacritics (ă â ê ô + tone marks) instead of digits.
    HKL usKeyboard = (HKL)(ULONG_PTR)0x04090409;

    hr = profileMgr->RegisterProfile(
        Globals::CLSID_TextService,
        Globals::TextServiceLangId,
        Globals::GUID_Profile,
        Globals::TextServiceDescription,
        (ULONG)wcslen(Globals::TextServiceDescription),
        dllPath,
        (ULONG)wcslen(dllPath),
        (ULONG)(-IDI_KTYPE),  // icon resource ID (negative = resource ID)
        usKeyboard,            // US keyboard as substitute layout
        0,     // preferred layout
        TRUE,  // enable by default
        0);    // flags

    return hr;
}

static HRESULT RegisterCategories() {
    ATL::CComPtr<ITfCategoryMgr> categoryMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr,
        CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&categoryMgr);
    if (FAILED(hr)) return hr;

    hr = categoryMgr->RegisterCategory(Globals::CLSID_TextService,
        GUID_TFCAT_TIP_KEYBOARD, Globals::CLSID_TextService);
    if (FAILED(hr)) return hr;

    hr = categoryMgr->RegisterCategory(Globals::CLSID_TextService,
        GUID_TFCAT_TIPCAP_UIELEMENTENABLED, Globals::CLSID_TextService);
    if (FAILED(hr)) return hr;

    hr = categoryMgr->RegisterCategory(Globals::CLSID_TextService,
        GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, Globals::CLSID_TextService);
    if (FAILED(hr)) return hr;

    hr = categoryMgr->RegisterCategory(Globals::CLSID_TextService,
        GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT, Globals::CLSID_TextService);
    if (FAILED(hr)) return hr;

    hr = categoryMgr->RegisterCategory(Globals::CLSID_TextService,
        GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, Globals::CLSID_TextService);

    return hr;
}

static void UnregisterProfiles() {
    ATL::CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr,
        CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&profileMgr))) {
        profileMgr->UnregisterProfile(Globals::CLSID_TextService,
            Globals::TextServiceLangId, Globals::GUID_Profile, 0);
    }

    // UnregisterProfile removes the profile but leaves the parent TIP key.
    // Manually delete it to prevent ghost IME entries after uninstall.
    std::wstring clsidStr = ClsidToString(Globals::CLSID_TextService);
    std::wstring tipPath = L"SOFTWARE\\Microsoft\\CTF\\TIP\\" + clsidStr;
    RegDeleteTreeW(HKEY_LOCAL_MACHINE, tipPath.c_str());
}

static void UnregisterCategories() {
    ATL::CComPtr<ITfCategoryMgr> categoryMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_CategoryMgr, nullptr,
        CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&categoryMgr))) {
        categoryMgr->UnregisterCategory(Globals::CLSID_TextService,
            GUID_TFCAT_TIP_KEYBOARD, Globals::CLSID_TextService);
        categoryMgr->UnregisterCategory(Globals::CLSID_TextService,
            GUID_TFCAT_TIPCAP_UIELEMENTENABLED, Globals::CLSID_TextService);
        categoryMgr->UnregisterCategory(Globals::CLSID_TextService,
            GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, Globals::CLSID_TextService);
        categoryMgr->UnregisterCategory(Globals::CLSID_TextService,
            GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT, Globals::CLSID_TextService);
        categoryMgr->UnregisterCategory(Globals::CLSID_TextService,
            GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, Globals::CLSID_TextService);
    }
}

} // namespace KType

// Standard COM DLL registration exports
STDAPI DllRegisterServer() {
    // Step 1: Register COM server (CLSID -> DLL path)
    HRESULT hr = KType::RegisterCOMServer();
    if (FAILED(hr)) {
        DllUnregisterServer();
        return hr;
    }

    // Step 2: Register TSF profile
    hr = KType::RegisterProfiles();
    if (FAILED(hr)) {
        DllUnregisterServer();
        return hr;
    }

    // Step 3: Register TSF categories
    hr = KType::RegisterCategories();
    if (FAILED(hr)) {
        DllUnregisterServer();
        return hr;
    }

    return S_OK;
}

STDAPI DllUnregisterServer() {
    KType::UnregisterProfiles();
    KType::UnregisterCategories();
    KType::UnregisterCOMServer();
    return S_OK;
}
