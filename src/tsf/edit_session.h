#pragma once
#include "pch.h"

namespace KType {

// Generic edit session that invokes a callback within a TfEditCookie context.
// TSF requires all text modifications to happen inside an edit session.
class EditSession : public ITfEditSession {
public:
    using Callback = std::function<HRESULT(TfEditCookie ec)>;

    EditSession(Callback callback) : _callback(std::move(callback)), _refCount(1) {}

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override {
        if (riid == IID_IUnknown || riid == IID_ITfEditSession) {
            *ppvObj = static_cast<ITfEditSession*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObj = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++_refCount; }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG r = --_refCount;
        if (r == 0) delete this;
        return r;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec) override {
        return _callback(ec);
    }

    // Helper: request an edit session on a context
    static HRESULT Request(ITfContext* context, Callback callback, DWORD flags = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE) {
        auto* session = new EditSession(std::move(callback));
        HRESULT hrSession;
        HRESULT hr = context->RequestEditSession(TfClientId(0), session, flags, &hrSession);
        session->Release();
        return SUCCEEDED(hr) ? hrSession : hr;
    }

private:
    Callback _callback;
    ULONG _refCount;
};

} // namespace KType
