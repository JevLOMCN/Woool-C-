#include "StdAfx.h"
#include ".\aiobjectex.h"
#include ".\logicmap.h"
#include "server.h"
#include "downitemobject.h"
#include "humanplayer.h"

#define	_SIGN(n) (n>0?1:(n<0?-1:0))

CAiObjectEx::CAiObjectEx(void)
{
	m_bLastActionWasAttack = FALSE;//边走边打,移动跟攻击开关
	m_wHomeX = 0; // 出生X坐标
	m_wHomeY = 0; // 出生Y坐标
	m_boBackHome = FALSE;
	m_wGoGomeTime = 0;
}

CAiObjectEx::~CAiObjectEx(void)
{
}

BOOL CAiObjectEx::Init(int x, int y)
{
	m_wHomeX = x;
	m_wHomeY = y;
	return TRUE;
}

VOID CAiObjectEx::Ai_KeepLine(BYTE nAttackDistance, DWORD dwFlag)
{
	CAliveObject* pTarget = GetTarget();
	if (pTarget == nullptr)
	{
		Ai_StupidMove();
		return;
	}
	int dx = (int)getX() - pTarget->getX();
	int dy = (int)getY() - pTarget->getY();
	BYTE nDist = DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY());
	if (nDist > nAttackDistance) GotoPosition(pTarget->getX(), pTarget->getY());
	else if ((dwFlag & LINEATTACK_XSTYLE) && abs(dx) == abs(dy))
		AttackTarget();
	else if ((dwFlag & LINEATTACK_CROSS) && (dx == 1 || dy == 1))
		AttackTarget();
	else
	{
		int nx, ny;
		//	move to line
		UINT Nx = 0, Ny = 0;
		UINT Mx = 0, My = 0;
		if (dwFlag & LINEATTACK_XSTYLE)
		{
			int p = min(abs(dx), abs(dy));
			if (p == 0)p = max(abs(dx), abs(dy));
			nx = p * (dx == 0 ? (Getrand(2) == 0 ? 1 : -1) : _SIGN(dx));
			ny = p * (dy == 0 ? (Getrand(2) == 0 ? 1 : -1) : _SIGN(dy));
			Nx = (int)pTarget->getX() + nx;
			Ny = (int)pTarget->getY() + ny;
		}
		if (dwFlag & LINEATTACK_CROSS)
		{
			if (abs(dx) > abs(dy))
			{
				Mx = getX();
				My = pTarget->getY();
			}
			else
			{
				Mx = pTarget->getX();
				My = getY();
			}
		}
		//if( DISTANCE( nX, nY, Nx, Ny ) < DISTANCE( nX, nY, Mx, My ) )
			//GotoPosition( Nx, Ny );
		//else
		GotoPosition(Mx, My);
	}
}

VOID CAiObjectEx::Ai_Escape(BYTE nEscapeDistance)
{
	CAliveObject* pTarget = GetTarget();
	if (pTarget == nullptr)
	{
		Ai_StupidMove();
		return;
	}
	BYTE nDist = DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY());
	if (nDist < nEscapeDistance)
		GoAwayPosition(pTarget->getX(), pTarget->getY());
}

VOID CAiObjectEx::Ai_Follow(BYTE nAttackDistance)
{
	CAliveObject* pTarget = GetTarget();
	if (pTarget == nullptr)
	{
		Ai_StupidMove();
		return;
	}
	BYTE nDist = DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY());
	if (nDist <= nAttackDistance && AttackTarget()) return;
	GotoPosition(pTarget->getX(), pTarget->getY());
}

VOID CAiObjectEx::Ai_Static(BYTE nAttackDistance, BOOL bAutoHide)
{
	CAliveObject* pTarget = GetTarget();
	if (pTarget)
	{
		if (bAutoHide)
		{
			if (IsSystemFlagSeted(SF_HIDED))
			{
				SetSystemFlag(SF_HIDED, FALSE);
				DWORD dwView[4] = { GetFeather(), 0, 0, 0 };
				SendAroundMsg(GetId(), 0x14, getX(), getY(), GetDirection(), dwView, sizeof(dwView));//怪显现封包
				char szBuffer[1024];
				int length = 0;
				if (GetViewmsg(szBuffer, length)) SendAroundMsg(szBuffer, length);
				SetAction(AT_SHOW, GetDirection(), getX(), getY(), 1400);
				return;
			}
		}
		BYTE nDist = DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY());
		if (nDist <= nAttackDistance) AttackTarget();
	}
	else
	{
		CHumanPlayer* pOwner = (CHumanPlayer*)GetOwner();
		if (pOwner != nullptr) //如果没目标的情况下,获取主人的目标,这里应该给魔法师的宠物用
		{
			CLogicMap* pMap = pOwner->GetMap();
			if (pMap != nullptr)
			{
				int x = pOwner->getX(), y = pOwner->getY();
				if (GetMap() == pOwner->GetMap() && DISTANCE(getX(), getY(), x, y) >= VIEW_RANGE)
				{
					pOwner->PetsFlyto(pMap, x, y, FALSE);
					return;
				}
			}
		}
		if (bAutoHide)
		{
			if (!IsSystemFlagSeted(SF_HIDED))
			{
				SendAroundMsg(GetId(), 0x15, getX(), getY(), (WORD)GetDirection());//怪隐藏封包
				AddProcess(EP_SETSYSTEMFLAG, SF_HIDED, TRUE, 0, 0, 700);
				SetAction(AT_HIDED, GetDirection(), getX(), getY(), 700);
				return;
			}
		}
	}
}

VOID CAiObjectEx::Ai_KeepDistance(BYTE nAttackDistance, BYTE nEscapeDistance)
{
	CAliveObject* pTarget = GetTarget();
	if (pTarget == nullptr)
	{
		Ai_StupidMove();
		return;
	}
	//需要距离
	BYTE nDist = DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY());
	if (nDist > nAttackDistance)
		GotoPosition(pTarget->getX(), pTarget->getY());
	else if (nDist <= nEscapeDistance)
		GoAwayPosition(pTarget->getX(), pTarget->getY());
	else if (!AttackTarget())
		GotoPosition(pTarget->getX(), pTarget->getY());
}

VOID CAiObjectEx::Ai_BianZouBianDa(BYTE nAttackDistance)
{
	CAliveObject* pTarget = GetTarget();
	if (pTarget == nullptr)
	{
		Ai_StupidMove();
		return;
	}
	BYTE nDist = DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY());//获取目标与自己的距离
	if (nDist <= 1 && AttackTarget())//如果与目标距离为攻击距离内就不移动, 直接攻击
		return;
	if (nDist <= nAttackDistance && !m_bLastActionWasAttack) //如果上一次不是攻击, 则优先攻击
	{
		if (AttackTarget())
		{
			m_bLastActionWasAttack = TRUE; //标记本次为攻击
			return;
		}
	}
	GotoPosition(pTarget->getX(), pTarget->getY());
	m_bLastActionWasAttack = FALSE;
}

VOID CAiObjectEx::Ai_BackHome(BYTE nAttackDistance, BYTE btViewDistance, MONSTERGEN* pGen)
{
	if (pGen == nullptr) return;
	CAliveObject* pTarget = GetTarget();
	xIndexPtrListHelper<CMonsterEx> helper(pGen->xMonsterList);
	std::vector<CMonsterEx*> monsterList; // 获取阵型中所有怪物
	monsterList.clear();
	monsterList.reserve(pGen->curcount); // 预分配内存
	for (CMonsterEx* pMonster = helper.first(); pMonster; pMonster = helper.next())
	{
		monsterList.push_back(pMonster);
	}
	if (m_boBackHome) // 回防状态
	{
		if (!IsHome()) // 还没回到家
		{
			if (m_wGoGomeTime > btViewDistance) // 立即飞回出生点（黑袍长老阵的特点）
			{
				m_wGoGomeTime = 0;
				FlyTo(GetMap(), m_wHomeX, m_wHomeY);
			}
			else
			{
				GotoPosition(m_wHomeX, m_wHomeY);
				m_wGoGomeTime++;
			}
		}
		else // 已经回到家
		{
			BOOL shouldReturn = TRUE;
			for (CMonsterEx* pMonster : monsterList) // 检查是否所有都回到家
			{
				if (!pMonster->IsHome())
				{
					shouldReturn = FALSE;
					break;
				}
			}
			if (!shouldReturn) return; // 如果不是所有都回到家, 直接返回, 继续等待
			for (CMonsterEx* pMonster : monsterList) // 让还没有回家的飞回家, 并取消回防状态
			{
				pMonster->SetTarget(nullptr);
				pMonster->m_boBackHome = FALSE;
			}
		}
	}
	if (pTarget == nullptr) // 没有目标, 在出生点附近巡逻
	{
		BYTE nCenterDist = DISTANCE(getX(), getY(), pGen->x, pGen->y);
		if (nCenterDist <= nAttackDistance)
			Ai_StupidMove();
		else
			GotoPosition(m_wHomeX, m_wHomeY);
	}
	else // 有目标, 进行警戒逻辑
	{
		std::vector<std::pair<CMonsterEx*, BYTE>> monsterDistances;
		for (CMonsterEx* pMonster : monsterList) // 计算所有怪物到目标的距离
		{
			BYTE dist = DISTANCE(pMonster->getX(), pMonster->getY(), pTarget->getX(), pTarget->getY());
			monsterDistances.push_back({ pMonster, dist });
		}
		// 按距离排序
		std::sort(monsterDistances.begin(), monsterDistances.end(),
			[](const auto& a, const auto& b) {
				return a.second < b.second;
			});
		// 最近怪物的距离
		BYTE nDist = monsterDistances[0].second;
		if (nDist <= nAttackDistance) // 在攻击范围内
		{
			if (monsterDistances[0].first == this) // 协同攻击：让最近的怪物发起攻击
			{
				size_t attackCount = MIN(monsterDistances.size(), (size_t)12);
				for (size_t i = 0; i < attackCount; i++)
				{
					monsterDistances[i].first->AttackTarget();
				}
			}
		}
		else // 在视野内但攻击范围外, 向目标移动
		{
			BYTE nDistFirst = DISTANCE(monsterDistances[0].first->getX(), monsterDistances[0].first->getY(), pGen->x, pGen->y);
			if (nDistFirst > btViewDistance) // 如果已经跟随到距离中心点远大于视觉距离了, 立即回防
			{
				if (monsterDistances[0].first == this)
				{
					for (CMonsterEx* pMonster : monsterList)
					{
						pMonster->SetTarget(nullptr);
						pMonster->m_wGoGomeTime = 0;
						pMonster->m_boBackHome = TRUE;
					}
				}
			}
			else
				GotoPosition(pTarget->getX(), pTarget->getY());
		}
	}
}

VOID CAiObjectEx::Ai_StupidMove()
{
	if (m_pMap == nullptr)return;
	CHumanPlayer* pOwner = (CHumanPlayer*)GetOwner();
	if (pOwner)
	{
		if (IsGotoOwner()) return;
		CLogicMap* pMap = pOwner->GetMap();
		int x = pOwner->getX(), y = pOwner->getY();
		if (GetMap() == pOwner->GetMap() && DISTANCE(getX(), getY(), x, y) >= VIEW_RANGE)
			pOwner->PetsFlyto(pMap, x, y, FALSE);
		else
		{
			GETNEXTPOS(x, y, (pOwner->GetDirection() + 4) % 8);
			if (!m_pMap->IsBlocked(x, y))
				GotoPosition(x, y);
			else if (DISTANCE(getX(), getY(), x, y) > 1)
				GotoPosition(x, y);
		}
		return;
	}
	if (Getrand(150) == 0) // 这个判断是让每一个怪走动有时间差
	{
		int start = Getrand(8); // 随机一个方向
		for (int i = 0; i < 8; i++) // 寻找一个可以走动的方向
		{
			if (AiWalk((i + start) % 8)) break;
		}
	}
}

BOOL CAiObjectEx::AttackTarget(e_direction dir)
{
	return TRUE;
}

VOID CAiObjectEx::GoAwayPosition(int nX, int nY)
{
	int x = nX - getX();
	int y = nY - getY();
	GotoPosition(getX() - x, getY() - y);
}

VOID CAiObjectEx::GotoPosition(int nX, int nY)
{
	int nDir = DR_DOWN;
	int nCurrX = m_wX;
	int nCurrY = m_wY;
	// 判断目标位置是否在当前对象的水平线、垂直线或对角线上.
	bool bOnLineOrDiagonal = false;
	if (nX == nCurrX || nY == nCurrY || (nX - nY) == (nCurrX - nCurrY) || (nX + nY) == (nCurrX + nCurrY))
		bOnLineOrDiagonal = true;
	// 如果目标位置不在水平线、垂直线或对角线上, 则选择优先移动方向.
	if (!bOnLineOrDiagonal)
	{
		if (abs(nCurrX - nX) > abs(nCurrY - nY)) // 如果 X 方向的距离差较大, 优先水平移动.
			nCurrX = (nX > nCurrX) ? nCurrX + 1 : nCurrX - 1;
		else // 如果 Y 方向的距离差较大或相等, 优先垂直移动.
			nCurrY = (nY > nCurrY) ? nCurrY + 1 : nCurrY - 1;
	}
	nDir = GetDir8(nX, nY, nCurrX, nCurrY);
	//是否需要跑
	UINT nDist = DISTANCE(nCurrX, nCurrY, nX, nY);
	BOOL bCheckRun = (nDist > 2);
	for (int i = 0; i < 7; i++)
	{
		if (!AiWalk(nDir, bCheckRun))
		{
			if (Getrand(3))
				nDir++;
			else if (nDir > 0)
				nDir--;
			else
				nDir = 7;
			if (nDir > 7)
				nDir = 0;
		}
		else
			break;
	}
}

CAliveObject* CAiObjectEx::SearchTarget(BYTE btTargetSelect, BYTE btViewDistance)
{
	switch (btTargetSelect)//目标选择
	{
	case ATS_LEVEL:
	{
		return SearchTargetProp(btViewDistance, PI_LEVEL);
	}
	break;
	case ATS_HP:
	{
		return SearchTargetProp(btViewDistance, PI_CURHP);
	}
	break;
	case ATS_DISTANCE:
	default:
	{
		return SearchTargetDistance(btViewDistance);
	}
	break;
	}
	return nullptr;
}

CAliveObject* CAiObjectEx::SearchTargetDistance(BYTE btViewDistance)
{
	if (m_xVisibleObjectList.getCount() == 0) return nullptr;
	const UINT nX = getX();
	const UINT nY = getY();
	const BOOL bIsCrazy = IsSystemFlagSeted(SF_CRAZY);
	CAliveObject* pTarget = nullptr;
	UINT nMinDistance = UINT_MAX;
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = m_xVisibleObjectList.getHead();
	while (pNode != nullptr)
	{
		VISIBLE_OBJECT* pvObj = pNode->getObject();
		pNode = pNode->getNext();
		if (pvObj == nullptr) continue;
		CMapObject* pMapObj = pvObj->pObject;
		if (pMapObj == nullptr) continue;
		if (pMapObj->GetClassType() != CLS_ALIVEOBJECT) continue;
		CAliveObject* pObject = static_cast<CAliveObject*>(pMapObj);
		const UINT objX = pObject->getX();
		const UINT objY = pObject->getY();
		int dx = abs((int)nX - (int)objX);
		if (dx > btViewDistance) continue;
		int dy = abs((int)nY - (int)objY);
		if (dy > btViewDistance) continue;
		UINT nDist = (dx > dy) ? (UINT)dx : (UINT)dy;
		if (nDist > btViewDistance) continue;
		if (pObject->IsDeath()) continue;
		if (!bIsCrazy)
		{
			if (!IsProperTarget(pObject)) continue;
			if (!IsTargetSelectable(pObject)) continue;
		}
		if (nDist < nMinDistance)
		{
			nMinDistance = nDist;
			pTarget = pObject;
		}
	}
	return pTarget;
}

CAliveObject* CAiObjectEx::SearchTargetProp(BYTE btViewDistance, int prop)
{
	if (m_xVisibleObjectList.getCount() == 0) return nullptr;
	const UINT nX = getX();
	const UINT nY = getY();
	const BOOL bIsCrazy = IsSystemFlagSeted(SF_CRAZY);
	const PROP_INDEX propIndex = static_cast<PROP_INDEX>(prop);

	CAliveObject* pTarget = nullptr;
	UINT nMinPropValue = UINT_MAX;
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = m_xVisibleObjectList.getHead();
	while (pNode != nullptr)
	{
		VISIBLE_OBJECT* pvObj = pNode->getObject();
		pNode = pNode->getNext();
		if (pvObj == nullptr) continue;
		CMapObject* pMapObj = pvObj->pObject;
		if (pMapObj == nullptr) continue;
		if (pMapObj->GetClassType() != CLS_ALIVEOBJECT) continue;
		CAliveObject* pObject = static_cast<CAliveObject*>(pMapObj);
		int dx = abs((int)nX - (int)pObject->getX());
		if (dx > btViewDistance) continue;
		int dy = abs((int)nY - (int)pObject->getY());
		if (dy > btViewDistance) continue;
		if ((dx > dy ? dx : dy) > btViewDistance) continue;
		if (pObject->IsDeath()) continue;
		if (!bIsCrazy)
		{
			if (!IsProperTarget(pObject)) continue;
			if (!IsTargetSelectable(pObject)) continue;
		}
		UINT nPropValue = pObject->GetPropValue(propIndex);
		if (nPropValue < nMinPropValue)
		{
			nMinPropValue = nPropValue;
			pTarget = pObject;
		}
	}
	return pTarget;
}

CAliveObject* CAiObjectEx::SearchTargetForOwner(BYTE btViewDistance)
{
	if (m_xVisibleObjectList.getCount() == 0) return nullptr;
	CAliveObject* pOwner = GetOwner();
	if (pOwner == nullptr) return nullptr;
	const UINT nX = getX();
	const UINT nY = getY();
	const BOOL bIsCrazy = IsSystemFlagSeted(SF_CRAZY);
	const e_object_type ownerType = pOwner->GetType();
	CAliveObject* pTarget = nullptr;
	UINT nMinDistance = UINT_MAX;
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = m_xVisibleObjectList.getHead();
	while (pNode != nullptr)
	{
		VISIBLE_OBJECT* pvObj = pNode->getObject();
		pNode = pNode->getNext();
		if (pvObj == nullptr) continue;
		CMapObject* pMapObj = pvObj->pObject;
		if (pMapObj == nullptr) continue;
		if (pMapObj->GetClassType() != CLS_ALIVEOBJECT) continue;
		CAliveObject* pObject = static_cast<CAliveObject*>(pMapObj);
		int dx = abs((int)nX - (int)pObject->getX());
		if (dx > btViewDistance) continue;
		int dy = abs((int)nY - (int)pObject->getY());
		if (dy > btViewDistance) continue;
		UINT nDist = (dx > dy) ? (UINT)dx : (UINT)dy;
		if (nDist > btViewDistance) continue;
		if (pObject->IsDeath()) continue;
		if (!bIsCrazy)
		{
			if (!IsTargetSelectable(pObject)) continue;
			if (!IsProperTarget(pObject)) continue;
			if (pObject->GetType() == ownerType) continue;
		}
		CAliveObject* pTestTarget = pObject->GetTarget();
		if (pTestTarget != nullptr)
		{
			if (pTestTarget == this) continue;
			if (pTestTarget == pOwner) continue;
			CAliveObject* pTargetOwner = pTestTarget->GetOwner();
			if (pTargetOwner != nullptr && pTargetOwner != pOwner) continue;
		}
		if (nDist < nMinDistance)
		{
			nMinDistance = nDist;
			pTarget = pObject;
		}
	}
	return pTarget;
}

CAliveObject* CAiObjectEx::SearchFriendDistance(BYTE nDistance)
{
	CLogicMap* pMap = GetMap();
	if (!pMap) return nullptr;
	int nDir = GetDirection();
	// 预计算方向偏移量
	int myX = getX(), myY = getY();
	int dirX = G_DIROFS[nDir % 8].x;
	int dirY = G_DIROFS[nDir % 8].y;
	int sideDirX = G_DIROFS[(nDir + 2) % 8].x;
	int sideDirY = G_DIROFS[(nDir + 2) % 8].y;
	// 搜索朝向的以攻击距离为半径的半圆内的对象
	for (int depth = 1; depth <= nDistance; depth++)
	{
		// 计算当前深度的中心点位置（沿方向前进depth步）
		int centerX = myX + dirX * depth;
		int centerY = myY + dirY * depth;
		// 横向扫描范围（形成半圆）
		int scanWidth = depth;
		for (int offset = -scanWidth; offset <= scanWidth; offset++)
		{
			// 计算垂直方向的偏移
			int sideX, sideY;
			if (offset >= 0) // 向左侧偏移
			{
				sideX = centerX + sideDirX * offset;
				sideY = centerY + sideDirY * offset;
			}
			else // 向右侧偏移（offset为负）
			{
				sideX = centerX - sideDirX * (-offset);
				sideY = centerY - sideDirY * (-offset);
			}
			CAliveObject* pAliveObj = pMap->FindTarget(this, sideX, sideY, FALSE, FALSE);
			if (pAliveObj && pAliveObj->GetType() == OBJ_MONSTER && !IsProperTarget(pAliveObj))
				return pAliveObj;
		}
	}
	return nullptr;
}

CMapObject* CAiObjectEx::SearchFitableItem(WORD x, WORD y)
{
	if (m_xVisibleItemsList.getCount() == 0) return nullptr;
	if (m_pMap == nullptr) return nullptr;
	CLogicMap* pMap = m_pMap;
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = m_xVisibleItemsList.getHead();
	while (pNode != nullptr)
	{
		VISIBLE_OBJECT* pvObject = pNode->getObject();
		pNode = pNode->getNext();
		if (pvObject == nullptr) continue;
		CMapObject* pObject = pvObject->pObject;
		if (pObject == nullptr) continue;
		if (pObject->GetType() != OBJ_DOWNITEM) continue;
		const WORD itemX = pObject->getX();
		const WORD itemY = pObject->getY();
		if (x != itemX) continue;
		if (y != itemY) continue;
		if (pMap->IsBlocked(itemX, itemY)) continue;
		return pObject;
	}
	return nullptr;
}

CMapObject* CAiObjectEx::SearchGodItem() // 搜索金币
{
	if (m_xVisibleItemsList.getCount() == 0) return nullptr;
	DWORD dwOwnerId = 0;
	CAliveObject* pOwner = GetOwner();
	if (pOwner != nullptr && pOwner->GetType() == OBJ_PLAYER)
		dwOwnerId = static_cast<CHumanPlayer*>(pOwner)->GetDBId();
	if (m_pMap == nullptr) return nullptr;
	CLogicMap* pMap = m_pMap;
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = m_xVisibleItemsList.getHead();
	while (pNode != nullptr)
	{
		VISIBLE_OBJECT* pvObject = pNode->getObject();
		pNode = pNode->getNext();
		if (pvObject == nullptr) continue;
		CMapObject* pObject = pvObject->pObject;
		if (pObject == nullptr) continue;
		if (pObject->GetType() != OBJ_DOWNITEM) continue;
		CDownItemObject* pDownItem = static_cast<CDownItemObject*>(pObject);
		const WORD itemX = pDownItem->getX();
		const WORD itemY = pDownItem->getY();
		if (pMap->IsBlocked(itemX, itemY)) continue;
		if (dwOwnerId > 0 && pDownItem->GetOwnerId() != dwOwnerId) continue;
		const ITEM& item = pDownItem->GetItem();
		if ((item.dwMakeIndex & 0x80000000) == 0) continue;
		if (item.baseitem.btStdMode != 255) continue;
		if (item.baseitem.btShape != 0) continue;
		return pObject;
	}
	return nullptr;
}