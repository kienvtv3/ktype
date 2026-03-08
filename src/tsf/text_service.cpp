#include "pch.h"
#include "text_service.h"
#include "context_manager.h"
#include "display_attribute.h"

namespace KType {

TextService::TextService() {
}

STDMETHODIMP TextService::Activate(ITfThreadMgr* threadMgr, TfClientId clientId) {
    return ActivateEx(threadMgr, clientId, 0);
}

STDMETHODIMP TextService::ActivateEx(ITfThreadMgr* threadMgr, TfClientId clientId, DWORD) {
    _threadMgr = threadMgr;
    _clientId = clientId;

    _contextMgr = new ContextManager();
    HRESULT hr = _contextMgr->Initialize(threadMgr, clientId);
    if (FAILED(hr)) {
        delete _contextMgr;
        _contextMgr = nullptr;
        return hr;
    }

    return S_OK;
}

STDMETHODIMP TextService::Deactivate() {
    if (_contextMgr) {
        _contextMgr->Uninitialize();
        _contextMgr->Release();
        _contextMgr = nullptr;
    }
    _threadMgr.Release();
    _clientId = 0;
    return S_OK;
}

STDMETHODIMP TextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo** ppEnum) {
    *ppEnum = new KType::EnumDisplayAttributeInfo();
    return S_OK;
}

STDMETHODIMP TextService::GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo** ppInfo) {
    if (IsEqualGUID(guid, Globals::GUID_DisplayAttribute)) {
        *ppInfo = new DisplayAttributeInfo();
        return S_OK;
    }
    *ppInfo = nullptr;
    return E_INVALIDARG;
}

} // namespace KType
