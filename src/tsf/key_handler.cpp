#include "pch.h"
#include "context_manager.h"
#include "key_translator.h"
#include "edit_session.h"

namespace KType {

STDMETHODIMP ContextManager::OnTestKeyDown(ITfContext* tfContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
    *pfEaten = FALSE;

    BYTE keyState[256];
    if (!GetKeyboardState(keyState)) return S_OK;

    Context* ctx = GetOrCreateContext(tfContext);
    if (ctx->IsBlocked()) return S_OK;

    // Ctrl/Alt modifiers: end composition then pass through (VietType behavior)
    // Uses async edit session — TSF executes immediately if document available
    if (KeyTranslator::HasModifiers(keyState)) {
        if (ctx->IsComposing()) {
            auto* session = new EditSession([ctx](TfEditCookie ec) -> HRESULT {
                return ctx->EndCompositionNow(ec);
            });
            HRESULT hrSession;
            tfContext->RequestEditSession(_clientId, session,
                TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
            session->Release();
        }
        return S_OK;
    }

    // Backspace: eat if we have pending input
    if (wParam == VK_BACK) {
        *pfEaten = ctx->HasPendingInput() ? TRUE : FALSE;
        return S_OK;
    }

    // Commit keys (Enter/Tab/Esc): eat if we need to commit first
    if (KeyTranslator::IsCommitKey(wParam)) {
        *pfEaten = ctx->HasPendingInput() ? TRUE : FALSE;
        return S_OK;
    }

    // Edit/navigation keys: commit first if needed, then pass through
    if (KeyTranslator::IsEditKey(wParam)) {
        // We'll commit in OnKeyDown, don't eat
        return S_OK;
    }

    // Translate to character (noChangeState=true to avoid corrupting dead key state)
    wchar_t ch = KeyTranslator::VkToChar(wParam, lParam, keyState, true);

    // Alphabetic keys that the Telex engine accepts
    if (ch && KeyTranslator::IsKeyEaten(ch)) {
        *pfEaten = TRUE;
        return S_OK;
    }

    // Non-alphabetic printable char (space, punctuation, underscore, digits)
    // If we have pending input, we need to commit first
    if (ch && ctx->HasPendingInput()) {
        *pfEaten = TRUE;
        return S_OK;
    }

    return S_OK;
}

STDMETHODIMP ContextManager::OnKeyDown(ITfContext* tfContext, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
    *pfEaten = FALSE;

    BYTE keyState[256];
    if (!GetKeyboardState(keyState)) return S_OK;

    // Modifiers: composition already committed in OnTestKeyDown, pass through
    if (KeyTranslator::HasModifiers(keyState)) return S_OK;

    Context* ctx = GetOrCreateContext(tfContext);
    if (ctx->IsBlocked()) return S_OK;

    // Backspace
    if (wParam == VK_BACK && ctx->HasPendingInput()) {
        auto* session = new EditSession([ctx](TfEditCookie ec) -> HRESULT {
            return ctx->ProcessBackspace(ec);
        });
        HRESULT hrSession;
        tfContext->RequestEditSession(_clientId, session,
            TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        session->Release();
        *pfEaten = TRUE;
        return S_OK;
    }

    // Commit keys: commit composition then pass key through
    if (KeyTranslator::IsCommitKey(wParam) && ctx->HasPendingInput()) {
        auto* session = new EditSession([ctx](TfEditCookie ec) -> HRESULT {
            return ctx->CommitComposition(ec);
        });
        HRESULT hrSession;
        tfContext->RequestEditSession(_clientId, session,
            TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        session->Release();
        // Don't eat the key — let Enter/Tab/Esc pass through to the app
        *pfEaten = FALSE;
        return S_OK;
    }

    // Edit/navigation keys: commit if needed
    if (KeyTranslator::IsEditKey(wParam) && ctx->HasPendingInput()) {
        auto* session = new EditSession([ctx](TfEditCookie ec) -> HRESULT {
            return ctx->CommitComposition(ec);
        });
        HRESULT hrSession;
        tfContext->RequestEditSession(_clientId, session,
            TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        session->Release();
        *pfEaten = FALSE;
        return S_OK;
    }

    wchar_t ch = KeyTranslator::VkToChar(wParam, lParam, keyState);
    if (!ch) return S_OK;

    // Alphabetic keys the Telex engine accepts
    if (KeyTranslator::IsKeyEaten(ch)) {
        auto* session = new EditSession([ctx, ch](TfEditCookie ec) -> HRESULT {
            return ctx->ProcessKey(ec, ch);
        });
        HRESULT hrSession;
        tfContext->RequestEditSession(_clientId, session,
            TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        session->Release();
        *pfEaten = TRUE;
        return S_OK;
    }

    // Non-alphabetic printable char with pending input → commit + inject char
    if (ctx->HasPendingInput()) {
        auto* session = new EditSession([ctx, ch](TfEditCookie ec) -> HRESULT {
            return ctx->CommitAndInsertChar(ec, ch);
        });
        HRESULT hrSession;
        tfContext->RequestEditSession(_clientId, session,
            TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        session->Release();
        *pfEaten = TRUE;  // We handled everything (commit + inject)
        return S_OK;
    }

    return S_OK;
}

STDMETHODIMP ContextManager::OnTestKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* pfEaten) {
    *pfEaten = FALSE;
    return S_OK;
}

STDMETHODIMP ContextManager::OnKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* pfEaten) {
    *pfEaten = FALSE;
    return S_OK;
}

} // namespace KType
