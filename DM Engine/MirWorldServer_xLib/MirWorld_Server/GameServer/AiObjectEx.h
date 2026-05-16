#pragma once
#include "aliveobject.h"

class CAiObjectEx : public CAliveObject
{
public:
	CAiObjectEx(void);
	virtual ~CAiObjectEx(void);
protected:
	BOOL Init(int x, int y);
	virtual BOOL AiWalk(int dir, BOOL bCheckRun = FALSE) { return FALSE; }
	CAliveObject* SearchTarget(BYTE btTargetSelect, BYTE btViewDistance);
	CAliveObject* SearchTargetDistance(BYTE btViewDistance);
	CAliveObject* SearchTargetProp(BYTE btViewDistance, int prop);
	CAliveObject* SearchTargetForOwner(BYTE btViewDistance);
	CAliveObject* SearchFriendDistance(BYTE nDistance);
	virtual BOOL AttackTarget(e_direction dir = ED_MAX);
	VOID Ai_KeepLine(BYTE nAttackDistance, DWORD dwFlag = LINEATTACK_XSTYLE | LINEATTACK_CROSS);
	VOID Ai_Escape(BYTE nEscapeDistance);
	VOID Ai_Follow(BYTE nAttackDistance);
	VOID Ai_Static(BYTE nAttackDistance, BOOL bAutoHide);
	VOID Ai_KeepDistance(BYTE nAttackDistance, BYTE nEscapeDistance);
	VOID Ai_BianZouBianDa(BYTE nAttackDistance);
	VOID Ai_BackHome(BYTE nAttackDistance, BYTE btViewDistance, MONSTERGEN* pGen);
	VOID Ai_StupidMove();
	BOOL IsHome() 
	{
		if (getX() == m_wHomeX && getY() == m_wHomeY)
			return TRUE;
		return FALSE;
	}
	//远离目标位置
	VOID GoAwayPosition(int nX, int nY);
	//靠近目标位置
	VOID GotoPosition(int nX, int nY);
	int GetDir8(int iTX, int iTY, int iOX, int iOY)
	{
		int x1, y1, j;
		// 计算距离坐标距离
		x1 = (iTX - iOX);
		y1 = -(iTY - iOY);
		//正方向,比如:0,4,8,12也就是北, 东, 南, 西
		if (x1 == 0)
		{
			if (y1 > 0)
				return 0;
			else
				return 4;
		}
		if (y1 == 0)
		{
			if (x1 > 0)
				return 2;
			else
				return 6;
		}
		//非正方向,比如东北, 东南...
		j = abs((int)((float)y1 / (float)x1 * 1000));
		if (x1 > 0)
		{
			if (y1 > 0)
			{
				if (j < 414)
					return 2;
				else if (j < 2414)
					return 1;
				else
					return 0;
			}
			else
			{
				if (j < 414)
					return 2;
				else if (j < 2414)
					return 3;
				else
					return 4;
			}
		}
		else
		{
			if (y1 > 0)
			{
				if (j < 414)
					return 6;
				else if (j < 2414)
					return 7;
				else
					return 0;
			}
			else
			{
				if (j < 414)
					return 6;
				else if (j < 2414)
					return 5;
				else
					return 4;
			}
		}
	}
	CMapObject* SearchFitableItem(WORD x, WORD y);
	CMapObject* SearchGodItem();
	virtual BOOL IsGotoOwner() { return TRUE; }//是否回到主人身边
	virtual BOOL IsTargetSelectable(CAliveObject* pTarget) { return TRUE; }
private:
	BOOL m_bLastActionWasAttack;//边走边打,移动跟攻击开关
	WORD m_wHomeX; // 出生X坐标
	WORD m_wHomeY; // 出生Y坐标
	BOOL m_boBackHome; // 是否回家
	WORD m_wGoGomeTime; // 回家次数
};