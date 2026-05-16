#include "stdafx.h"
#include "FengHaoGrowManager.h"
#include "humanplayer.h"

VOID CHumanPlayer::SendFengHaoGrowInfo()
{
	xPacket packet3(2048);
	xPacket packet5(1024);

	int nCount = m_FenghaoInfo.GetCount();
	packet3.push(&nCount, 4);
	packet5.push(&nCount, 4);

	DWORD dwNow = (DWORD)time(nullptr);
	CFengHaoGrowManager* pMgr = CFengHaoGrowManager::GetInstance();

	for (int i = 1; i < MAX_FENGHAO; ++i)
	{
		if (m_FenghaoInfo.mFengHaoRow[i].boActivation)
		{
			packet3.push(&i, 4);
			FengHaoGrowItem* pConfig = pMgr->GetItem(i);
			if (pConfig && pConfig->btLastDay > 0 && m_FenghaoInfo.mFengHaoRow[i].dwLastDate > 0)
			{
				int nLastDate = m_FenghaoInfo.mFengHaoRow[i].dwLastDate - dwNow;
				if (nLastDate > 0)
				{
					nLastDate = (int)(nLastDate / 3600);
					packet5.push(&nLastDate, 4);
				}
				else
					packet5.push(4);
			}
			else
				packet5.push(4);
		}
	}
	packet3.push((LPVOID)packet5.getbuf(), packet5.getsize());
	int nType1 = (int)m_FenghaoInfo.btType1;
	packet3.push(&nType1, 4);
	int nTypeVal = 0;
	if (m_FenghaoInfo.btType2 > 0)
		nTypeVal = (int)m_FenghaoInfo.btType2;
	else
		nTypeVal = (int)m_FenghaoInfo.btType3;
	packet3.push(&nTypeVal, 4);
	SendMsg(GetId(), 0x9b0, 0, 0, 0, (LPVOID)packet3.getbuf(), packet3.getsize());
}

VOID CHumanPlayer::SendFengHaoEquip(int nCount)
{
	if (nCount <= 0) return;
	FengHaoGrowItem* pConfig = CFengHaoGrowManager::GetInstance()->GetItem(nCount);
	if (!pConfig) return;
	BOOL boEquip = TRUE; // 是否是穿戴
	BYTE btOldType1 = 0; // 旧的穿戴封号
	BYTE btOldType23 = 0; // 旧的穿戴封号
	if (pConfig->btType >= 1)
	{
		if (pConfig->btType == 1)
		{
			btOldType23 = m_FenghaoInfo.btType2;
			// 特殊封号、节日封号只能同时展示一个
			if (m_FenghaoInfo.btType2 == 0)
			{
				m_FenghaoInfo.btType3 = 0; // 互斥处理
				m_FenghaoInfo.btType2 = (BYTE)nCount;
				SendMsg(GetId(), 0x9b0, 1, nCount, 0);
			}
			else
			{
				// 当前激活的封号就是本ID，则卸下当前封号
				if (m_FenghaoInfo.btType2 == (BYTE)nCount)
				{
					m_FenghaoInfo.btType2 = 0;
					SendMsg(GetId(), 0x9b0, 1, 0, 0); // 卸下
					boEquip = FALSE;
				}
				else
				{
					// 替换当前封号
					m_FenghaoInfo.btType3 = 0;
					m_FenghaoInfo.btType2 = (BYTE)nCount;
					SendMsg(GetId(), 0x9b0, 1, nCount, 0);
				}
			}
		}
		else if (pConfig->btType == 2)
		{
			btOldType23 = m_FenghaoInfo.btType3;
			if (m_FenghaoInfo.btType3 == 0)
			{
				m_FenghaoInfo.btType2 = 0; // 互斥处理
				m_FenghaoInfo.btType3 = (BYTE)nCount;
				SendMsg(GetId(), 0x9b0, 1, nCount, 0);
			}
			else
			{
				if (m_FenghaoInfo.btType3 == (BYTE)nCount)
				{
					m_FenghaoInfo.btType3 = 0;
					SendMsg(GetId(), 0x9b0, 1, 0, 0);
					boEquip = FALSE;
				}
				else
				{
					m_FenghaoInfo.btType2 = 0;
					m_FenghaoInfo.btType3 = (BYTE)nCount;
					SendMsg(GetId(), 0x9b0, 1, nCount, 0);
				}
			}
		}
	}
	else
	{
		btOldType1 = m_FenghaoInfo.btType1;
		// 普通封号处理
		if (m_FenghaoInfo.btType1 == (BYTE)nCount)
		{
			m_FenghaoInfo.btType1 = 0;
			SendMsg(GetId(), 0x9b0, 1, 0, 0);
			boEquip = FALSE;
		}
		else
		{
			m_FenghaoInfo.btType1 = (BYTE)nCount;
			SendMsg(GetId(), 0x9b0, 1, nCount, 0);
		}
	}
	if (btOldType1 > 0)
		RecalcFengHaoProp(btOldType1, FALSE, FALSE);//卸下旧封号的属性
	if (btOldType23 > 0)
		RecalcFengHaoProp(btOldType23, FALSE, FALSE);//卸下旧封号的属性
	// 刷新时长封号增加的属性值
	RecalcFengHaoProp(nCount, boEquip);
	UpdateViewName();    // 刷新显示名称
	SendFengHaoGrowInfo(); // 刷新界面信息
}

VOID CHumanPlayer::SendFengHaoData()
{
	char szBuffer[12910]{};
	int size = EncodeMsgRaw(szBuffer, GetId(), 0x9b0, 3, 0, 0, (LPVOID)g_sData.c_str(), (int)g_sData.length());
	OnAroundMsg(this, szBuffer, size);
}

VOID CHumanPlayer::RecalcFengHaoProp(BYTE index, BOOL boOperate, BOOL boProp)
{
	FengHaoGrowItem* pConfig = CFengHaoGrowManager::GetInstance()->GetItem(index);
	for (int i = 0; i < pConfig->nAttrCnt; i++)
	{
		switch (pConfig->attrs[i].nAttrType)
		{
		case 9: //物理防御
		{
			if (boOperate)
			{
				AddProp(PI_MINAC, pConfig->attrs[i].nValue1);
				AddProp(PI_MAXAC, pConfig->attrs[i].nValue2);
			}
			else
			{
				DecProp(PI_MINAC, pConfig->attrs[i].nValue1);
				DecProp(PI_MAXAC, pConfig->attrs[i].nValue2);
			}
		}
		break;
		case 10: //魔法防御
		{
			if (boOperate)
			{
				AddProp(PI_MINMAC, pConfig->attrs[i].nValue1);
				AddProp(PI_MAXMAC, pConfig->attrs[i].nValue2);
			}
			else
			{
				DecProp(PI_MINMAC, pConfig->attrs[i].nValue1);
				DecProp(PI_MAXMAC, pConfig->attrs[i].nValue2);
			}
		}
		break;
		case 100: //仅战士增加的血量
		{
			if (GetPro() != PRO_WARRIOR) break;
			if (boOperate)
			{
				AddProp(PI_MAXHP, pConfig->attrs[i].nValue1);
			}
			else
			{
				DecProp(PI_MAXHP, pConfig->attrs[i].nValue1);
			}
		}
		break;
		case 101: //仅法师增加的血量
		{
			if (GetPro() != PRO_MAGICIAN) break;
			if (boOperate)
			{
				AddProp(PI_MAXHP, pConfig->attrs[i].nValue1);
			}
			else
			{
				DecProp(PI_MAXHP, pConfig->attrs[i].nValue1);
			}
		}
		break;
		case 102: //仅道士增加的血量
		{
			if (GetPro() != PRO_TAOSHI) break;
			if (boOperate)
			{
				AddProp(PI_MAXHP, pConfig->attrs[i].nValue1);
			}
			else
			{
				DecProp(PI_MAXHP, pConfig->attrs[i].nValue1);
			}
		}
		break;
		case 105: //用攻击自动转换为本职业的攻击类型
		{
			switch (GetPro())
			{
			case PRO_WARRIOR:
			{
				if (boOperate)
				{
					AddProp(PI_MINDC, pConfig->attrs[i].nValue1);
					AddProp(PI_MAXDC, pConfig->attrs[i].nValue2);
				}
				else
				{
					DecProp(PI_MINDC, pConfig->attrs[i].nValue1);
					DecProp(PI_MAXDC, pConfig->attrs[i].nValue2);
				}
			}
			break;
			case PRO_MAGICIAN:
			{
				if (boOperate)
				{
					AddProp(PI_MINMC, pConfig->attrs[i].nValue1);
					AddProp(PI_MAXMC, pConfig->attrs[i].nValue2);
				}
				else
				{
					DecProp(PI_MINMC, pConfig->attrs[i].nValue1);
					DecProp(PI_MAXMC, pConfig->attrs[i].nValue2);
				}
			}
			break;
			case PRO_TAOSHI:
			{
				if (boOperate)
				{
					AddProp(PI_MINSC, pConfig->attrs[i].nValue1);
					AddProp(PI_MAXSC, pConfig->attrs[i].nValue2);
				}
				else
				{
					DecProp(PI_MINSC, pConfig->attrs[i].nValue1);
					DecProp(PI_MAXSC, pConfig->attrs[i].nValue2);
				}
			}
			break;
			}
		}
		break;
		}
	}
	if (boProp)
	{
		UpdateProp();
		UpdateSubProp();
	}
}

VOID CHumanPlayer::OnFengHaoInfo(FenghaoInfo* pInfo)
{
	m_FenghaoInfo = *pInfo;
	BOOL bChanged = FALSE;
	if (m_FenghaoInfo.btType1 > 0) 
	{ 
		RecalcFengHaoProp(m_FenghaoInfo.btType1, TRUE, FALSE); bChanged = TRUE; 
	}
	if (m_FenghaoInfo.btType2 > 0) 
	{ 
		RecalcFengHaoProp(m_FenghaoInfo.btType2, TRUE, FALSE); bChanged = TRUE; 
	}
	if (m_FenghaoInfo.btType3 > 0) 
	{ 
		RecalcFengHaoProp(m_FenghaoInfo.btType3, TRUE, FALSE); bChanged = TRUE; 
	}
	if (bChanged)
	{
		UpdateProp();
		UpdateSubProp();
	}
}

VOID CHumanPlayer::CheckFengHaoTimeOut()
{
	CFengHaoGrowManager* pMgr = CFengHaoGrowManager::GetInstance();
	DWORD dwNow = (DWORD)time(nullptr);
	for (int i = 1; i < MAX_FENGHAO; ++i)
	{
		FengHaoGrowItem* pConfig = pMgr->GetItem(i);
		FengHaoRow pFengHaoRow = m_FenghaoInfo.mFengHaoRow[i];
		if (pConfig && pConfig->btLastDay != 0 && pFengHaoRow.boActivation) //只处理有时间限制的封号
		{
			if (pFengHaoRow.dwLastDate <= dwNow)
			{
				SendMsg(GetId(), 0x9b0, 1, 0, 0); // 卸下
				RecalcFengHaoProp(i, FALSE);
			}
		}
	}
}

VOID CHumanPlayer::UpdateFengHaoToDB()
{
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
	if (pObj)
		pObj->UpdateFengHaoInfo(GetDBId(), &m_FenghaoInfo);
}
