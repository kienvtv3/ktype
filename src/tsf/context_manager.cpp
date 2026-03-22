#include "pch.h"
#include "context_manager.h"
#include "Globals.h"

namespace KType {

ContextManager::ContextManager() {}

ContextManager::~ContextManager() {
    Uninitialize();
}

HRESULT ContextManager::Initialize(ITfThreadMgr* threadMgr, TfClientId clientId) {
    _threadMgr = threadMgr;
    _clientId = clientId;

    // Advise thread manager event sink
    ATL::CComPtr<ITfSource> source;
    HRESULT hr = _threadMgr->QueryInterface(&source);
    if (SUCCEEDED(hr)) {
        hr = source->AdviseSink(IID_ITfThreadMgrEventSink,
            static_cast<ITfThreadMgrEventSink*>(this), &_threadMgrEventSinkCookie);
    }
    if (FAILED(hr)) return hr;

    // Advise key event sink
    ATL::CComPtr<ITfKeystrokeMgr> keystrokeMgr;
    hr = _threadMgr->QueryInterface(&keystrokeMgr);
    if (SUCCEEDED(hr)) {
        hr = keystrokeMgr->AdviseKeyEventSink(_clientId,
            static_cast<ITfKeyEventSink*>(this), TRUE);
    }

    return hr;
}

void ContextManager::Uninitialize() {
    if (_threadMgr) {
        ATL::CComPtr<ITfKeystrokeMgr> keystrokeMgr;
        if (SUCCEEDED(_threadMgr->QueryInterface(&keystrokeMgr))) {
            keystrokeMgr->UnadviseKeyEventSink(_clientId);
        }
    }

    if (_threadMgr && _threadMgrEventSinkCookie != TF_INVALID_COOKIE) {
        ATL::CComPtr<ITfSource> source;
        if (SUCCEEDED(_threadMgr->QueryInterface(&source))) {
            source->UnadviseSink(_threadMgrEventSinkCookie);
        }
        _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
    }

    for (auto& pair : _contextMap) {
        pair.second->Release();
    }
    _contextMap.clear();
    _threadMgr.Release();
}

// IUnknown
STDMETHODIMP ContextManager::QueryInterface(REFIID riid, void** ppvObj) {
    if (riid == IID_IUnknown || riid == IID_ITfThreadMgrEventSink) {
        *ppvObj = static_cast<ITfThreadMgrEventSink*>(this);
    } else if (riid == IID_ITfKeyEventSink) {
        *ppvObj = static_cast<ITfKeyEventSink*>(this);
    } else {
        *ppvObj = nullptr;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) ContextManager::AddRef() { return ++_refCount; }
STDMETHODIMP_(ULONG) ContextManager::Release() {
    ULONG r = --_refCount;
    if (r == 0) delete this;
    return r;
}

// ITfThreadMgrEventSink
STDMETHODIMP ContextManager::OnInitDocumentMgr(ITfDocumentMgr*) { return S_OK; }
STDMETHODIMP ContextManager::OnUninitDocumentMgr(ITfDocumentMgr*) { return S_OK; }
STDMETHODIMP ContextManager::OnSetFocus(ITfDocumentMgr* docMgr, ITfDocumentMgr* prevDocMgr) {
    // When focus moves to a different document, commit pending composition on old document
    if (prevDocMgr && prevDocMgr != docMgr) {
        ATL::CComPtr<ITfContext> prevContext;
        if (SUCCEEDED(prevDocMgr->GetTop(&prevContext)) && prevContext) {
            auto it = _contextMap.find(prevContext);
            if (it != _contextMap.end() && it->second->HasPendingInput()) {
                it->second->RequestCommit();
            }
        }
    }
    return S_OK;
}
STDMETHODIMP ContextManager::OnPushContext(ITfContext*) { return S_OK; }

STDMETHODIMP ContextManager::OnPopContext(ITfContext* context) {
    auto it = _contextMap.find(context);
    if (it != _contextMap.end()) {
        if (it->second->HasPendingInput()) {
            it->second->RequestCommit();
        }
        it->second->Release();
        _contextMap.erase(it);
    }
    return S_OK;
}

// ITfKeyEventSink
STDMETHODIMP ContextManager::OnSetFocus(BOOL fForeground) {
    // When thread loses foreground (e.g., Alt+Tab), commit all pending compositions
    if (!fForeground) {
        for (auto& pair : _contextMap) {
            if (pair.second->HasPendingInput()) {
                pair.second->RequestCommit();
            }
        }
    }
    return S_OK;
}
STDMETHODIMP ContextManager::OnPreservedKey(ITfContext*, REFGUID, BOOL* pfEaten) {
    *pfEaten = FALSE;
    return S_OK;
}

Context* ContextManager::GetOrCreateContext(ITfContext* tfContext) {
    auto it = _contextMap.find(tfContext);
    if (it != _contextMap.end()) return it->second;

    auto* ctx = new Context(tfContext, _clientId);
    _contextMap[tfContext] = ctx;
    return ctx;
}

} // namespace KType
