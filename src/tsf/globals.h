#pragma once
#include "pch.h"

namespace KType {
namespace Globals {

// KType COM Class ID
// {7E8F1A2B-3C4D-5E6F-A1B2-C3D4E5F67890}
extern const CLSID CLSID_TextService;

// KType TSF Profile GUID
// {8F9A2B3C-4D5E-6F7A-B2C3-D4E5F6789012}
extern const GUID GUID_Profile;

// Display attribute GUID
// {9A0B3C4D-5E6F-7A8B-C3D4-E5F678901234}
extern const GUID GUID_DisplayAttribute;

extern const wchar_t* TextServiceDescription;
extern const wchar_t* ConfigKeyName;
extern const LANGID TextServiceLangId;
extern HINSTANCE DllInstance;

} // namespace Globals
} // namespace KType
