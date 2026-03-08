#pragma once
#include "pch.h"
#include "telex.h"
#include "globals.h"

namespace KType {

class Context : public ITfCompositionSink {
public:
    Context(ITfContext* tfContext, TfClientId clientId);
    ~Context();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // ITfCompositionSink
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ec, ITfComposition* composition) override;

    HRESULT ProcessKey(TfEditCookie ec, wchar_t ch);
    HRESULT ProcessBackspace(TfEditCookie ec);
    HRESULT CommitComposition(TfEditCookie ec);
    HRESULT CommitAndInsertChar(TfEditCookie ec, wchar_t ch);
    HRESULT CancelComposition(TfEditCookie ec);

    bool IsComposing() const { return _composition != nullptr; }
    bool HasPendingInput() const { return !_engine.IsEmpty(); }
    ITfContext* GetTfContext() const { return _tfContext; }
    HRESULT RequestCommit();
    bool IsBlocked() const { return _blocked; }

private:
    HRESULT StartComposition(TfEditCookie ec);
    HRESULT UpdateCompositionText(TfEditCookie ec, const std::wstring& text);
    HRESULT EndComposition(TfEditCookie ec, const std::wstring& finalText);

    ULONG _refCount = 1;
    ATL::CComPtr<ITfContext> _tfContext;
    ATL::CComPtr<ITfComposition> _composition;
    TfClientId _clientId;
    TelexEngine _engine;
    bool _blocked = false;
};

} // namespace KType
