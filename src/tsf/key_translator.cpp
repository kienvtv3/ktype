#include "pch.h"
#include "key_translator.h"
#include "telex.h"

namespace KType {

// Cache US English keyboard layout for ToUnicodeEx
// The Vietnamese keyboard layout maps number keys to diacritics (ă â ê ô + tones).
// KType does all Vietnamese processing itself, so we always use US layout underneath.
static HKL GetUSKeyboardLayout() {
    static HKL s_hkl = LoadKeyboardLayoutW(L"00000409", KLF_NOTELLSHELL);
    return s_hkl;
}

wchar_t KeyTranslator::VkToChar(WPARAM vk, LPARAM lParam, const BYTE* keyState, bool noChangeState) {
    wchar_t buf[4] = {};
    UINT scanCode = (UINT)((lParam >> 16) & 0xFF);
    // Flag 4: don't change internal keyboard state (avoids dead key issues with double ToUnicode calls)
    UINT flags = noChangeState ? 4 : 0;

    // Use US keyboard layout to avoid Vietnamese layout mapping numbers to diacritics
    HKL hkl = GetUSKeyboardLayout();
    int result = hkl
        ? ToUnicodeEx((UINT)vk, scanCode, keyState, buf, 4, flags, hkl)
        : ToUnicode((UINT)vk, scanCode, keyState, buf, 4, flags);
    if (result == 1) {
        return buf[0];
    }
    return 0;
}

bool KeyTranslator::IsKeyEaten(wchar_t ch) {
    return TelexEngine::AcceptsChar(ch);
}

bool KeyTranslator::IsCommitKey(WPARAM vk) {
    switch (vk) {
    case VK_RETURN:
    case VK_TAB:
    case VK_ESCAPE:
        return true;
    default:
        return false;
    }
}

bool KeyTranslator::IsEditKey(WPARAM vk) {
    switch (vk) {
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_HOME:
    case VK_END:
    case VK_DELETE:
        return true;
    default:
        return false;
    }
}

bool KeyTranslator::HasModifiers(const BYTE* keyState) {
    // Ctrl or Alt pressed (but not AltGr which is Ctrl+Alt)
    bool ctrl = (keyState[VK_CONTROL] & 0x80) != 0;
    bool alt = (keyState[VK_MENU] & 0x80) != 0;

    // AltGr = Ctrl+Alt, should be passed through for special chars
    if (ctrl && alt) return true;

    // Ctrl alone or Alt alone = modifier shortcut, don't eat
    if (ctrl || alt) return true;

    return false;
}

} // namespace KType
