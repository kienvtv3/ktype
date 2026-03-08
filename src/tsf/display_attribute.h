#pragma once
#include "pch.h"
#include "globals.h"

namespace KType {

// Display attribute for composition text (dotted underline)
class DisplayAttributeInfo : public ITfDisplayAttributeInfo {
public:
    DisplayAttributeInfo() : _refCount(1) {}

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override {
        if (riid == IID_IUnknown || riid == IID_ITfDisplayAttributeInfo) {
            *ppvObj = static_cast<ITfDisplayAttributeInfo*>(this);
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

    // ITfDisplayAttributeInfo
    STDMETHODIMP GetGUID(GUID* pguid) override {
        *pguid = Globals::GUID_DisplayAttribute;
        return S_OK;
    }
    STDMETHODIMP GetDescription(BSTR* pbstr) override {
        *pbstr = SysAllocString(L"KType Composition");
        return *pbstr ? S_OK : E_OUTOFMEMORY;
    }
    STDMETHODIMP GetAttributeInfo(TF_DISPLAYATTRIBUTE* pda) override {
        pda->crText.type = TF_CT_NONE;
        pda->crBk.type = TF_CT_NONE;
        pda->lsStyle = TF_LS_DOT;
        pda->fBoldLine = FALSE;
        pda->crLine.type = TF_CT_NONE;
        pda->bAttr = TF_ATTR_INPUT;
        return S_OK;
    }
    STDMETHODIMP SetAttributeInfo(const TF_DISPLAYATTRIBUTE*) override {
        return E_NOTIMPL;
    }
    STDMETHODIMP Reset() override {
        return S_OK;
    }

private:
    ULONG _refCount;
};

// Enumerator for display attributes (returns our single attribute)
class EnumDisplayAttributeInfo : public IEnumTfDisplayAttributeInfo {
public:
    EnumDisplayAttributeInfo() : _refCount(1), _index(0) {}

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) override {
        if (riid == IID_IUnknown || riid == IID_IEnumTfDisplayAttributeInfo) {
            *ppvObj = static_cast<IEnumTfDisplayAttributeInfo*>(this);
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

    STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo** ppEnum) override {
        auto* clone = new EnumDisplayAttributeInfo();
        clone->_index = _index;
        *ppEnum = clone;
        return S_OK;
    }
    STDMETHODIMP Next(ULONG count, ITfDisplayAttributeInfo** rgInfo, ULONG* pcFetched) override {
        ULONG fetched = 0;
        if (_index == 0 && count > 0) {
            rgInfo[0] = new DisplayAttributeInfo();
            fetched = 1;
            _index++;
        }
        if (pcFetched) *pcFetched = fetched;
        return (fetched == count) ? S_OK : S_FALSE;
    }
    STDMETHODIMP Reset() override { _index = 0; return S_OK; }
    STDMETHODIMP Skip(ULONG count) override { _index += count; return S_OK; }

private:
    ULONG _refCount;
    ULONG _index;
};

} // namespace KType
