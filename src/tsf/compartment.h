#pragma once
#include "pch.h"

namespace KType {

// Helper to read/write TSF compartment values
class Compartment {
public:
    static HRESULT GetBool(ITfThreadMgr* threadMgr, TfClientId /*clientId*/, REFGUID guid, bool& value) {
        ATL::CComPtr<ITfCompartmentMgr> compMgr;
        HRESULT hr = threadMgr->QueryInterface(&compMgr);
        if (FAILED(hr)) return hr;

        ATL::CComPtr<ITfCompartment> comp;
        hr = compMgr->GetCompartment(guid, &comp);
        if (FAILED(hr)) return hr;

        VARIANT var;
        hr = comp->GetValue(&var);
        if (FAILED(hr)) return hr;

        value = (var.vt == VT_I4) ? (var.lVal != 0) : false;
        return S_OK;
    }

    static HRESULT SetBool(ITfThreadMgr* threadMgr, TfClientId clientId, REFGUID guid, bool value) {
        ATL::CComPtr<ITfCompartmentMgr> compMgr;
        HRESULT hr = threadMgr->QueryInterface(&compMgr);
        if (FAILED(hr)) return hr;

        ATL::CComPtr<ITfCompartment> comp;
        hr = compMgr->GetCompartment(guid, &comp);
        if (FAILED(hr)) return hr;

        VARIANT var;
        var.vt = VT_I4;
        var.lVal = value ? 1 : 0;
        return comp->SetValue(clientId, &var);
    }
};

} // namespace KType
