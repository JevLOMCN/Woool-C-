#pragma once
class CFlagEventEx;
class CFlagEventListener
{
public:
	virtual VOID OnFlagSeted(CFlagEventEx* pEvent, UINT index, DWORD dwParam, DWORD dwTime) {}
	virtual VOID OnFlagClred(CFlagEventEx* pEvent, UINT index, DWORD dwParam, DWORD dwTime) {}
};