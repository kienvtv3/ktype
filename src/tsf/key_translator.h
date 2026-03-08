#pragma once
#include "pch.h"
#include "telex.h"

namespace KType {

class KeyTranslator {
public:
    // Translate a virtual key to a character. Returns 0 if not translatable.
    static wchar_t VkToChar(WPARAM vk, LPARAM lParam, const BYTE* keyState);

    // Should this key be consumed by the IME?
    static bool IsKeyEaten(wchar_t ch);

    // Is this key a composition-ending key?
    static bool IsCommitKey(WPARAM vk);

    // Is this an edit/navigation key?
    static bool IsEditKey(WPARAM vk);

    // Does this key have modifiers (Ctrl/Alt) that mean we should pass it through?
    static bool HasModifiers(const BYTE* keyState);
};

} // namespace KType
