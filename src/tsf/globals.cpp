#include "pch.h"
#include "globals.h"

namespace KType {
namespace Globals {

// {7E8F1A2B-3C4D-5E6F-A1B2-C3D4E5F67890}
const CLSID CLSID_TextService = {
    0x7e8f1a2b, 0x3c4d, 0x5e6f,
    { 0xa1, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6, 0x78, 0x90 }
};

// {8F9A2B3C-4D5E-6F7A-B2C3-D4E5F6789012}
const GUID GUID_Profile = {
    0x8f9a2b3c, 0x4d5e, 0x6f7a,
    { 0xb2, 0xc3, 0xd4, 0xe5, 0xf6, 0x78, 0x90, 0x12 }
};

// {9A0B3C4D-5E6F-7A8B-C3D4-E5F678901234}
const GUID GUID_DisplayAttribute = {
    0x9a0b3c4d, 0x5e6f, 0x7a8b,
    { 0xc3, 0xd4, 0xe5, 0xf6, 0x78, 0x90, 0x12, 0x34 }
};

const wchar_t* TextServiceDescription = L"KType";
const wchar_t* ConfigKeyName = L"Software\\KType";
const LANGID TextServiceLangId = MAKELANGID(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM);

HINSTANCE DllInstance = nullptr;

} // namespace Globals
} // namespace KType
