#include "StdAfx.h"
#include ".\scriptshell.h"
#include ".\scripttarget.h"
#include ".\scriptobject.h"
#include ".\scriptview.h"
#include ".\scriptelement.h"
#include <memory>

CScriptShell::CScriptShell(void)
{
	m_pScriptObject = nullptr;
	m_ExecuteResult = ER_OK;
	memset(&m_dwResultValue, 0, sizeof(m_dwResultValue));
	m_pCallParams = nullptr;
}

CScriptShell::~CScriptShell(void)
{
}

char* CScriptShell::GetVariableValue(const char* pszVariable)
{
	// $_param0 这样写, 就表示获取@carryto&159的第一个参数就是159
	if (_strnicmp(pszVariable, "param", 5) == 0)
	{
		UINT index = (UINT)StringToInteger(pszVariable + 5);
		if (index >= MAX_CALL_PARAMS)return nullptr;
		if (this->m_pCallParams == nullptr)return nullptr;
		return this->m_pCallParams[index].pszParam;
	}
	else
	{
		char* pvalue = m_xVarList.GetVarValue(pszVariable);
		if (pvalue != nullptr)
			return pvalue;
	}
	return xVariableProvider::GetVariableValue(pszVariable);
}

//	本函数返回FALSE后, 必须清空所有连接～
BOOL CScriptShell::Execute(CScriptTarget* pTarget, const char* pszPage1, BOOL bUserQuery, DWORD dwParam)
{
	CParamStackHelper paramstack;

	char szPage[1024];
	char* pszPage = nullptr;
	CallParamEx params[MAX_CALL_PARAMS];
	setExecuteResult(ER_OK);
	if (m_pScriptObject == nullptr)return FALSE;
	//	如果是第一次进入该SHELL
	BOOL bFirstPage = FALSE;
	auto view = std::make_unique<CScriptPageView>(this);
	if (pszPage1 == nullptr)
	{
		//	清空所有的连接
		pTarget->setCurScriptObject(nullptr);
		pTarget->setCurShell(nullptr);
		pszPage = "@main";
		bFirstPage = TRUE;
	}
	else
	{
		o_strncpy(szPage, pszPage1, 1020);
		xStringsExtracter<MAX_CALL_PARAMS + 1> queryparam(szPage, "&");
		for (UINT n = 1; n < queryparam.getCount(); n++)
		{
			params[n - 1] = queryparam[n];
			if (*params[n - 1].pszParam == '$')
			{
				char* p = pTarget->GetVariableValue(params[n - 1].pszParam + 1);
				if (p)
				{
					char* lparam = (char*)paramstack.getfreebuf();
					paramstack.push((LPVOID)p, (int)strlen(p) + 1);
					params[n - 1] = lparam;
				}
				else
					params[n - 1] = "";
			}
		}
		if (queryparam.getCount() > 1)
			this->m_pCallParams = params;
		pszPage = (char*)szPage;
	}
	//	如果当前SHELL不是本SHELL
	if (pTarget->getCurShell() != this)
	{
		//	连接到本SHELL, 清空页面连接
		pTarget->setCurScriptObject(nullptr);
		pTarget->setCurShell(this);
	}
	//	获取请求的页面
	CSe_Page* pPage = nullptr;
	//	如果当前脚本不是本地SHELL的脚本
	if (pTarget->getCurScriptObject() && pTarget->getCurScriptObject() != this->m_pScriptObject)
		pPage = pTarget->getCurScriptObject()->GetPage(pszPage); //	从远程脚本取得请求的页面
	else
		pPage = m_pScriptObject->GetPage(pszPage); //	从本地脚本取得请求的页面
	if (pPage == nullptr) return FALSE;
	//	如果页面没有访问权限, 则无法访问页面
	if (bUserQuery && pPage->getAccessRight() != PAR_PUBLIC) return FALSE;
	//	设置为当前页面
	pTarget->setCurScriptObject(pPage->getObject());
	//	执行页面
	if (!pPage->Execute(this, pTarget, view.get()))
	{
		//	执行结果是关闭页面, 就关闭页面
		if (getExecuteResult() == ER_CLOSE)
		{
			pTarget->SendClosePage(this);
			return TRUE;
		}
	}
	DWORD viewParam = view.get()->GetParam();
	if (viewParam != 0)
		dwParam = viewParam;
	//	呼叫派生类对特殊PAGEID的处理.
	if (!OnPageShow(pTarget, view.get(), pszPage)) return FALSE;
	//	否则, 发送页面
	view->SendPageToTarget(pTarget, dwParam);
	return TRUE;
}

BOOL CScriptShell::Execute(CScriptTarget* pTarget, CSe_Page* pPage, BOOL bUserQuery, DWORD dwParam)
{
	auto view = std::make_unique<CScriptPageView>(this);
	setExecuteResult(ER_OK);
	if (pPage == nullptr) return FALSE;
	//	如果页面没有访问权限, 则无法访问页面
	if (bUserQuery && pPage->getAccessRight() != PAR_PUBLIC) return FALSE;
	//	设置为当前页面
	pTarget->setCurScriptObject(pPage->getObject());
	//	执行页面
	if (!pPage->Execute(this, pTarget, view.get()))
	{
		//	执行结果是关闭页面, 就关闭页面
		if (getExecuteResult() == ER_CLOSE)
		{
			pTarget->SendClosePage(this);
			return TRUE;
		}
	}
	DWORD viewParam = view.get()->GetParam();
	if (viewParam != 0)
		dwParam = viewParam;
	//	否则, 发送页面
	view->SendPageToTarget(pTarget, dwParam);
	return TRUE;
}