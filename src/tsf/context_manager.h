#pragma once
#include "pch.h"
#include "context.h"

namespace KType {

class ContextManager : public ITfThreadMgrEventSink,
                       public ITfKeyEventSink {
public:
    ContextManager();
    ~ContextManager();

    HRESULT Initialize(ITfThreadMgr* threadMgr, TfClientId clientId);
    void Uninitialize();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr* docMgr) override;
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr* docMgr) override;
    STDMETHODIMP OnSetFocus(ITfDocumentMgr* docMgr, ITfDocumentMgr* prevDocMgr) override;
    STDMETHODIMP OnPushContext(ITfContext* context) override;
    STDMETHODIMP OnPopContext(ITfContext* context) override;

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground) override;
    STDMETHODIMP OnTestKeyDown(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
    STDMETHODIMP OnTestKeyUp(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
    STDMETHODIMP OnKeyDown(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
    STDMETHODIMP OnKeyUp(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) override;
    STDMETHODIMP OnPreservedKey(ITfContext* context, REFGUID rguid, BOOL* pfEaten) override;

private:
    Context* GetOrCreateContext(ITfContext* tfContext);

    ULONG _refCount = 1;
    ATL::CComPtr<ITfThreadMgr> _threadMgr;
    TfClientId _clientId = 0;
    DWORD _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
    std::map<ITfContext*, Context*> _contextMap;
};

} // namespace KType
