#include "pch.h"
#include "key_translator.h"
#include "telex.h"

namespace KType {

wchar_t KeyTranslator::VkToChar(WPARAM vk, LPARAM lParam, const BYTE* keyState) {
    wchar_t buf[4] = {};
    UINT scanCode = (UINT)((lParam >> 16) & 0xFF);
    int result = ToUnicode((UINT)vk, scanCode, keyState, buf, 4, 0);
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
