#include "pch.h"
#include "globals.h"
#include "resource.h"
#include <AccCtrl.h>
#include <AclAPI.h>
#include <strsafe.h>
#include <array>

namespace KType {

static constexpr DWORD ILOT_UNINSTALL = 0x00000001;
using InstallLayoutOrTip_t = BOOL(CALLBACK*)(LPCWSTR psz, DWORD dwFlags);
using SetDefaultLayoutOrTip_t = BOOL(CALLBACK*)(LPCWSTR psz, DWORD dwFlags);

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

    // hklSubstitute = NULL: don't register a substitute keyboard layout (VietType behavior)
    // Number key mapping is handled in code via ToUnicodeEx with US HKL instead
    hr = profileMgr->RegisterProfile(
        Globals::CLSID_TextService,
        Globals::TextServiceLangId,
        Globals::GUID_Profile,
        Globals::TextServiceDescription,
        (ULONG)wcslen(Globals::TextServiceDescription),
        dllPath,
        (ULONG)wcslen(dllPath),
        (ULONG)(-IDI_KTYPE),  // icon resource ID (negative = resource ID)
        NULL,  // no substitute layout (US layout handled by ToUnicodeEx)
        0,     // preferred layout
        TRUE,  // enable by default
        0);    // flags

    return hr;
}

// Task 4: Register categories including GUID_TFCAT_TIPCAP_COMLESS
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

    // Required for WoW (Windows on Windows) 32-bit apps
    hr = categoryMgr->RegisterCategory(Globals::CLSID_TextService,
        GUID_TFCAT_TIPCAP_COMLESS, Globals::CLSID_TextService);
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

// Task 7: Dynamic category unregistration using EnumCategoriesInItem
static void UnregisterCategories() {
    ATL::CComPtr<ITfCategoryMgr> categoryMgr;
    if (FAILED(CoCreateInstance(CLSID_TF_CategoryMgr, nullptr,
        CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&categoryMgr))) {
        return;
    }

    ATL::CComPtr<IEnumGUID> registeredCategories;
    HRESULT hr = categoryMgr->EnumCategoriesInItem(
        Globals::CLSID_TextService, &registeredCategories);
    if (FAILED(hr)) return;

    GUID cat = {};
    ULONG fetched = 0;
    while (registeredCategories->Next(1, &cat, &fetched) == S_OK) {
        categoryMgr->UnregisterCategory(
            Globals::CLSID_TextService, cat, Globals::CLSID_TextService);
    }
}

// Map Vietnamese keyboard layout -> US English in registry (VietType behavior)
// Keys that KType doesn't eat pass through to the app, which uses the active
// keyboard layout. Without this substitution, the Vietnamese layout maps
// number keys to diacritics instead of digits.
static void SetKeyboardSubstitution() {
    HKEY hKey;
    LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, L"Keyboard Layout\\Substitutes",
        0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    if (result == ERROR_SUCCESS) {
        const wchar_t* usLayout = L"00000409";
        RegSetValueExW(hKey, L"0000042a", 0, REG_SZ,
            (const BYTE*)usLayout, (DWORD)(wcslen(usLayout) + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
    }
}

static void RemoveKeyboardSubstitution() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Keyboard Layout\\Substitutes",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"0000042a");
        RegCloseKey(hKey);
    }
}

// Task 5: Build TIP string "042A:{CLSID}{Profile}" for InstallLayoutOrTip
static std::wstring GetTipString() {
    // Format: "042A:{CLSID-GUID}{Profile-GUID}"
    // 4 hex digits + ':' + 38 char GUID + 38 char GUID + null = 82 chars
    std::array<wchar_t, 4 + 1 + 2 * 38 + 1> tipString = {};
    HRESULT hr = StringCchPrintfW(
        tipString.data(), tipString.size(), L"%04X:{%036X}{%036X}",
        Globals::TextServiceLangId, 0, 0);
    if (FAILED(hr)) return std::wstring();

    hr = StringFromGUID2(Globals::CLSID_TextService, &tipString[4 + 1], 39)
        ? S_OK : E_FAIL;
    if (FAILED(hr)) return std::wstring();

    hr = StringFromGUID2(Globals::GUID_Profile, &tipString[4 + 1 + 38], 39)
        ? S_OK : E_FAIL;
    if (FAILED(hr)) return std::wstring();

    *tipString.rbegin() = 0;
    return std::wstring(tipString.data());
}

// Task 5: Install/uninstall TIP and set as default input method
static HRESULT InstallTip(bool install) {
    HRESULT hr;
    HMODULE inputDll = NULL;
    InstallLayoutOrTip_t ilot = NULL;
    SetDefaultLayoutOrTip_t sdlot = NULL;

    auto tipString = GetTipString();
    if (tipString.empty()) {
        return E_FAIL;
    }

    inputDll = LoadLibraryExW(L"input.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!inputDll) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    ilot = reinterpret_cast<InstallLayoutOrTip_t>(
        GetProcAddress(inputDll, "InstallLayoutOrTip"));
    if (!ilot) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        FreeLibrary(inputDll);
        return hr;
    }

    sdlot = reinterpret_cast<SetDefaultLayoutOrTip_t>(
        GetProcAddress(inputDll, "SetDefaultLayoutOrTip"));
    if (!sdlot) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        FreeLibrary(inputDll);
        return hr;
    }

    if (!ilot(tipString.c_str(), install ? 0 : ILOT_UNINSTALL)) {
        FreeLibrary(inputDll);
        return E_FAIL;
    }

    if (install && !sdlot(tipString.c_str(), 0)) {
        // Rollback: uninstall the TIP we just installed
        ilot(tipString.c_str(), ILOT_UNINSTALL);
        FreeLibrary(inputDll);
        return E_FAIL;
    }

    FreeLibrary(inputDll);
    return S_OK;
}

// Task 6: Allow ALL APPLICATION PACKAGES to query settings key (for UWP apps)
static HRESULT SetSettingsKeyAcl() {
    HKEY hKey;
    LONG err = RegCreateKeyExW(HKEY_CURRENT_USER, Globals::ConfigKeyName,
        0, nullptr, 0, READ_CONTROL | WRITE_DAC, nullptr, &hKey, nullptr);
    if (err != ERROR_SUCCESS) return HRESULT_FROM_WIN32(err);

    // Get existing security descriptor
    std::vector<BYTE> sdbuf;
    DWORD sdbufsize = SECURITY_DESCRIPTOR_MIN_LENGTH * 2;
    for (int i = 0; i < 2; i++) {
        sdbuf.resize(sdbufsize);
        err = RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION,
            &sdbuf[0], &sdbufsize);
        if (err != ERROR_INSUFFICIENT_BUFFER) break;
    }
    if (err != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(err);
    }

    // Extract existing DACL
    PACL dacl;
    BOOL daclPresent, daclDefaulted;
    if (!GetSecurityDescriptorDacl(&sdbuf[0], &daclPresent, &dacl, &daclDefaulted)) {
        DWORD gle = GetLastError();
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(gle);
    }

    // Create ALL APPLICATION PACKAGES SID
    std::array<BYTE, SECURITY_MAX_SID_SIZE> sid = {};
    DWORD cbSid = static_cast<DWORD>(sid.size());
    if (!CreateWellKnownSid(WinBuiltinAnyPackageSid, NULL, &sid[0], &cbSid)) {
        DWORD gle = GetLastError();
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(gle);
    }

    // First revoke all access, then grant KEY_QUERY_VALUE only
    std::array<EXPLICIT_ACCESSW, 2> ea = {};

    ea[0].grfAccessPermissions = KEY_ALL_ACCESS;
    ea[0].grfAccessMode = REVOKE_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = reinterpret_cast<LPWCH>(&sid[0]);

    ea[1].grfAccessPermissions = KEY_QUERY_VALUE;
    ea[1].grfAccessMode = GRANT_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[1].Trustee.ptstrName = reinterpret_cast<LPWCH>(&sid[0]);

    PACL pNewAcl = nullptr;
    err = SetEntriesInAclW(static_cast<ULONG>(ea.size()), ea.data(), dacl, &pNewAcl);
    if (err != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(err);
    }

    // Build new security descriptor with the updated DACL
    std::vector<BYTE> newSdBuf(SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (!InitializeSecurityDescriptor(&newSdBuf[0], SECURITY_DESCRIPTOR_REVISION)) {
        DWORD gle = GetLastError();
        LocalFree(pNewAcl);
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(gle);
    }

    if (!SetSecurityDescriptorDacl(&newSdBuf[0], TRUE, pNewAcl, daclDefaulted)) {
        DWORD gle = GetLastError();
        LocalFree(pNewAcl);
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(gle);
    }

    err = RegSetKeySecurity(hKey, DACL_SECURITY_INFORMATION, &newSdBuf[0]);

    LocalFree(pNewAcl);
    RegCloseKey(hKey);

    if (err != ERROR_SUCCESS) return HRESULT_FROM_WIN32(err);
    return S_OK;
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

    // Step 4: Map Vietnamese keyboard layout to US English
    KType::SetKeyboardSubstitution();

    // Step 5: Set settings key ACL for UWP app access
    KType::SetSettingsKeyAcl();

    // Step 6: Install and set as default input method
    KType::InstallTip(true);

    return S_OK;
}

STDAPI DllUnregisterServer() {
    // Uninstall TIP before removing registration
    KType::InstallTip(false);

    KType::RemoveKeyboardSubstitution();
    KType::UnregisterProfiles();
    KType::UnregisterCategories();
    KType::UnregisterCOMServer();
    return S_OK;
}
