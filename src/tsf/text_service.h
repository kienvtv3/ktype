#pragma once
#include "pch.h"
#include "globals.h"

namespace KType {

class ContextManager;

class ATL_NO_VTABLE TextService
    : public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>
    , public ATL::CComCoClass<TextService, &Globals::CLSID_TextService>
    , public ITfTextInputProcessorEx
    , public ITfDisplayAttributeProvider {
public:
    TextService();

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(TextService)
        COM_INTERFACE_ENTRY(ITfTextInputProcessor)
        COM_INTERFACE_ENTRY(ITfTextInputProcessorEx)
        COM_INTERFACE_ENTRY(ITfDisplayAttributeProvider)
    END_COM_MAP()

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr* threadMgr, TfClientId clientId) override;
    STDMETHODIMP Deactivate() override;

    // ITfTextInputProcessorEx
    STDMETHODIMP ActivateEx(ITfThreadMgr* threadMgr, TfClientId clientId, DWORD flags) override;

    // ITfDisplayAttributeProvider
    STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo** ppEnum) override;
    STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo** ppInfo) override;

private:
    ATL::CComPtr<ITfThreadMgr> _threadMgr;
    TfClientId _clientId = 0;
    ContextManager* _contextMgr = nullptr;
};

OBJECT_ENTRY_AUTO(Globals::CLSID_TextService, TextService)

} // namespace KType
