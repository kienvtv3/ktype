#include "pch.h"
#include "context.h"
#include "edit_session.h"
#include "globals.h"

namespace KType {

Context::Context(ITfContext* tfContext, TfClientId clientId)
    : _tfContext(tfContext), _clientId(clientId) {
}

Context::~Context() {
}

STDMETHODIMP Context::QueryInterface(REFIID riid, void** ppvObj) {
    if (riid == IID_IUnknown || riid == IID_ITfCompositionSink) {
        *ppvObj = static_cast<ITfCompositionSink*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObj = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) Context::AddRef() { return ++_refCount; }
STDMETHODIMP_(ULONG) Context::Release() {
    ULONG r = --_refCount;
    if (r == 0) delete this;
    return r;
}

STDMETHODIMP Context::OnCompositionTerminated(TfEditCookie, ITfComposition*) {
    _composition.Release();
    _engine.Reset();
    return S_OK;
}

HRESULT Context::ProcessKey(TfEditCookie ec, wchar_t ch) {
    TelexStates state = _engine.PushChar(ch);

    std::wstring text;
    if (state == TelexStates::Valid) {
        text = _engine.Peek();
    } else if (state == TelexStates::Invalid) {
        text = _engine.RetrieveRaw();
    } else {
        return S_OK;
    }

    if (!IsComposing()) {
        HRESULT hr = StartComposition(ec);
        if (FAILED(hr)) return hr;
    }
    return UpdateCompositionText(ec, text);
}

HRESULT Context::ProcessBackspace(TfEditCookie ec) {
    if (!HasPendingInput()) return S_FALSE;

    _engine.Backspace();

    if (_engine.IsEmpty()) {
        if (IsComposing()) return EndComposition(ec, L"");
        return S_OK;
    }

    std::wstring preview = (_engine.GetState() == TelexStates::Valid)
        ? _engine.Peek() : _engine.RetrieveRaw();
    return UpdateCompositionText(ec, preview);
}

HRESULT Context::CommitComposition(TfEditCookie ec) {
    if (!HasPendingInput()) return S_OK;

    _engine.Commit();
    std::wstring result = _engine.Retrieve();

    HRESULT hr = IsComposing() ? EndComposition(ec, result) : S_OK;
    _engine.Reset();
    return hr;
}

HRESULT Context::CommitAndInsertChar(TfEditCookie ec, wchar_t ch) {
    if (!HasPendingInput()) {
        // No composition — just insert the char directly
        ATL::CComPtr<ITfInsertAtSelection> insertAtSel;
        HRESULT hr = _tfContext->QueryInterface(&insertAtSel);
        if (FAILED(hr)) return hr;
        wchar_t buf[2] = { ch, 0 };
        ATL::CComPtr<ITfRange> range;
        return insertAtSel->InsertTextAtSelection(ec, 0, buf, 1, &range);
    }

    // Commit engine and append the triggering char to the result
    // This avoids a two-step commit-then-insert which has selection state issues
    _engine.Commit();
    std::wstring result = _engine.Retrieve();
    result += ch;

    HRESULT hr = IsComposing() ? EndComposition(ec, result) : S_OK;
    _engine.Reset();
    return hr;
}

HRESULT Context::CancelComposition(TfEditCookie ec) {
    if (!HasPendingInput()) return S_OK;

    _engine.Cancel();
    std::wstring raw = _engine.Retrieve();

    HRESULT hr = IsComposing() ? EndComposition(ec, raw) : S_OK;
    _engine.Reset();
    return hr;
}

HRESULT Context::RequestCommit() {
    auto* session = new EditSession([this](TfEditCookie ec) -> HRESULT {
        return CommitComposition(ec);
    });
    HRESULT hrSession;
    HRESULT hr = _tfContext->RequestEditSession(_clientId, session,
        TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
    session->Release();
    return SUCCEEDED(hr) ? hrSession : hr;
}

HRESULT Context::StartComposition(TfEditCookie ec) {
    ATL::CComPtr<ITfContextComposition> contextComp;
    HRESULT hr = _tfContext->QueryInterface(&contextComp);
    if (FAILED(hr)) return hr;

    ATL::CComPtr<ITfInsertAtSelection> insertAtSel;
    hr = _tfContext->QueryInterface(&insertAtSel);
    if (FAILED(hr)) return hr;

    ATL::CComPtr<ITfRange> range;
    hr = insertAtSel->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, nullptr, 0, &range);
    if (FAILED(hr)) return hr;

    hr = contextComp->StartComposition(ec, range, this, &_composition);
    return hr;
}

HRESULT Context::UpdateCompositionText(TfEditCookie ec, const std::wstring& text) {
    if (!_composition) return E_UNEXPECTED;

    ATL::CComPtr<ITfRange> range;
    HRESULT hr = _composition->GetRange(&range);
    if (FAILED(hr)) return hr;

    hr = range->SetText(ec, 0, text.c_str(), (LONG)text.size());
    if (FAILED(hr)) return hr;

    // Move caret to end
    ATL::CComPtr<ITfRange> clonedRange;
    hr = range->Clone(&clonedRange);
    if (SUCCEEDED(hr)) {
        clonedRange->Collapse(ec, TF_ANCHOR_END);
        TF_SELECTION sel = {};
        sel.range = clonedRange;
        sel.style.ase = TF_AE_END;
        sel.style.fInterimChar = FALSE;
        _tfContext->SetSelection(ec, 1, &sel);
    }

    return S_OK;
}

HRESULT Context::EndComposition(TfEditCookie ec, const std::wstring& finalText) {
    if (!_composition) return S_OK;

    ATL::CComPtr<ITfRange> range;
    HRESULT hr = _composition->GetRange(&range);
    if (SUCCEEDED(hr)) {
        range->SetText(ec, 0, finalText.c_str(), (LONG)finalText.size());

        // Move caret to end of committed text
        // Without this, some apps (e.g. Explorer search bar) leave caret at start
        ATL::CComPtr<ITfRange> caretRange;
        if (SUCCEEDED(range->Clone(&caretRange))) {
            caretRange->Collapse(ec, TF_ANCHOR_END);
            TF_SELECTION sel = {};
            sel.range = caretRange;
            sel.style.ase = TF_AE_END;
            sel.style.fInterimChar = FALSE;
            _tfContext->SetSelection(ec, 1, &sel);
        }
    }

    _composition->EndComposition(ec);
    _composition.Release();
    return S_OK;
}

HRESULT Context::EndCompositionNow(TfEditCookie ec) {
    if (!_composition) return S_OK;

    // End composition without changing text — leave preview text as-is (VietType behavior)
    _composition->EndComposition(ec);
    _composition.Release();
    _engine.Reset();
    return S_OK;
}

} // namespace KType
