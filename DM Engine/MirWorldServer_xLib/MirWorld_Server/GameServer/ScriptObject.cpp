#include "StdAfx.h"
#include ".\scriptobject.h"
//#include ".\scriptsection.h"
#include ".\scriptelement.h"
#include ".\scriptobjectmgr.h"

CScriptObject::CScriptObject(void) : 
	m_xPageList(TRUE), m_iTradeItemCount(0), m_szFileName{}, m_szLeftPage{}, m_szName{}
{
	m_pPageList = nullptr;
	m_pGoodsList = nullptr;
	memset(m_btTradeItemType, 0, sizeof(m_btTradeItemType));
	this->m_pLeftPage = nullptr;
	this->m_bBigBox = FALSE;
}

CScriptObject::~CScriptObject(void)
{
}

VOID CScriptObject::Destroy()
{
	//for( UINT i = 0;i < this->m_xPageList.GetCount();i ++ )
	//{
	//	CSe_Page * pPage = (CSe_Page*)m_xPageList[i]->lpObject;
	//	if( pPage )delete pPage;
	//}
	if (m_pPageList)
	{
		m_pPageList->delToTail();
		m_pPageList = nullptr;
	}
	if (m_pGoodsList)
	{
		m_pGoodsList->delToTail();
		m_pGoodsList = nullptr;
	}
	m_xPageList.Clear();
	m_xVarList.ClearVars();
}

BOOL CScriptObject::Load(const char* pszFilename)
{
	CScriptFile file;
	if (!file.Load(pszFilename))
		return FALSE;
	if (pszFilename != this->m_szFileName)
		o_strncpy(this->m_szFileName, pszFilename, 1020);
	m_bBigBox = FALSE;
	this->m_pLeftPage = nullptr;
	char* pLine = file.FirstLine();
	do
	{
		if (pLine == nullptr)break;
		switch (*pLine)
		{
		case 0:
			continue;
		case '[':
		{
			char szName[200];
			filtercopy(szName, pLine, 7, CharSetWhite);
			if (_stricmp(szName, "[Goods]") == 0)//对比GOODS是否相等
			{
				CSe_Goods* pGoods = new CSe_Goods;
				if (!pGoods->Parse(file))
					delete pGoods;
				else
				{
					if (m_pGoodsList)
						m_pGoodsList->addTail(pGoods);
					else
						m_pGoodsList = pGoods;
				}
			}
			else
			{
				CSe_Page* pPage = new CSe_Page;
				if (!pPage->Parse(file))
					delete pPage;
				else
				{
					pPage->setObject(this);
					if (m_pPageList)
						m_pPageList->addTail(pPage);
					else
						m_pPageList = pPage;
					m_xPageList.Add(pPage->getName(), (LPVOID)pPage);
				}
			}
		}
		break;
		case '+':
		{
			AddTradeItemType((BYTE)StringToInteger(pLine + 1));
		}
		break;
		case '#':
		{
			if (_strnicmp(pLine, "#version:", 9) == 0)
			{
				pLine += 9;
				xStringsExtracter<4> version(pLine, ".", " \t");
				m_Version.dwVersion = 0;
				if (version[0])m_Version.v1 = StringToInteger(version[0]);
				if (version[1])m_Version.v2 = StringToInteger(version[1]);
				if (version[2])m_Version.v3 = StringToInteger(version[2]);
				if (version[3])m_Version.v4 = StringToInteger(version[3]);
			}
			else if (_strnicmp(pLine, "#var", 4) == 0)
			{
				char* Params[4];
				xCharSet csWht(" \t()\"=");
				xCharSet csSpl(" \t(),=");
				//char * Params[20];
				int nParam = ExtractStrings(pLine, csWht, csSpl, Params, 4, FALSE);
				if (nParam >= 3)
				{
					m_xVarList.AddVar(Params[1], Params[2]);
				}
			}
			else if (_strnicmp(pLine, "#bigbox:", 8) == 0)
			{
				m_bBigBox = TRUE;
				o_strncpy(m_szLeftPage, pLine + 8, 250);
			}
			else
				continue;
		}
		break;
		case ';':
			break;
		}
	} while (pLine = file.NextLine());
	_splitpath(pszFilename, nullptr, nullptr, m_szName, nullptr);
	return TRUE;
}

CSe_Page* CScriptObject::GetPage(const char* pszPage)
{
	char* pOName = (char*)pszPage;
	char* p = strchr(pOName, '.');
	if (p != nullptr)
	{
		if (*(p + 1) == 0)
			return nullptr;
		if (*pOName == '@')pOName++;
		*p = 0;
		CScriptObject* pObject = CScriptObjectMgr::GetInstance()->GetScriptObject(pOName);
		*p = '.';
		if (pObject == nullptr)return nullptr;
		char* pDot = p;
		if (*(p + 1) != '@')
			*p = '@';
		else
			p++;

		CSe_Page* pPage = pObject->GetPage(p);
		*pDot = '.';
		return pPage;
	}
	return (CSe_Page*)m_xPageList.ObjectOf(pszPage);
}

VOID CScriptObject::AddTradeItemType(BYTE btType)
{
	if (m_btTradeItemType[btType] == 0)
		m_btTradeItemType[btType] = 1;
}

BOOL CScriptObject::IsItemTradeble(BYTE btStdMode)
{
	return m_btTradeItemType[btStdMode];
}

VOID CScriptObject::Reload()
{
	Destroy();
	Load(this->m_szFileName);
}

CScriptObject* CScriptObject::GetLeftPage()
{
	if (!this->IsBigBox())return nullptr;
	if (this->m_pLeftPage == nullptr)
		this->m_pLeftPage = CScriptObjectMgr::GetInstance()->GetScriptObject(this->m_szLeftPage);
	return m_pLeftPage;
}