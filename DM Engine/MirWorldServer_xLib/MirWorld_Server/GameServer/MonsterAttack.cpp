#include "stdafx.h"
#include "MonsterAttack.h"
#include "monsterex.h"
#include "logicmap.h"
#include "aliveobject.h"
#include "localdefine.h"
#include "gameworld.h"
#include "BlueFireEvent.h"

VOID CMonsterAttack::BlackRobeHelper(CMonsterEx* pFriend)
{
	if (pFriend->CustomIsOutTime(5000)) // 黑袍加血辅助
	{
		if (!pFriend->IsProcess(EP_MAGHEALING) && !pFriend->IsDeath() && !pFriend->IsFullHp())
		{
			pFriend->CustomSaveTime();
			int nDamage = pMon->GetPropMC();
			pFriend->AddProcess(EP_MAGHEALING, nDamage, 0, 0, 0, 200);
			pMon->SendSkill(pFriend->GetId(), pFriend->getX(), pFriend->getY(), 0x2001);
		}
	}
	if (!pFriend->IsProcess(EM_RELIVE) && pFriend->IsDeath()) //黑袍复活辅助
	{
		if (pFriend->GetRevivalCount() < 3 && pFriend->BodyIsOutTime(5000))
		{
			int nHp = pFriend->GetPropValue(PI_MAXHP) / 2;
			pFriend->AddProcess(EM_RELIVE, nHp, 0, 0, 0, 200);
			pMon->SendSkill(pFriend->GetId(), pFriend->getX(), pFriend->getY(), 0x2002);
		}
	}
}

VOID CMonsterAttack::MoYan(int nDamage, damage_type damagetype)
{
	std::vector<CAliveObject*> validTargets;//存储所有符合条件的目标
	const int targetDir = pMon->GetDirection();
	const int myX = pMon->getX();
	const int myY = pMon->getY();
	const int attackDist = pMon->GetDesc()->attackdesc.AttackDistance;
	//搜索扇形范围内的所有目标
	const int nStartX = myX - attackDist;
	const int nStartY = myY - attackDist;
	const int nEndX = myX + attackDist;
	const int nEndY = myY + attackDist;

	bool bFoundEnough = false;
	for (int x = nStartX; x <= nEndX && !bFoundEnough; x++)
	{
		for (int y = nStartY; y <= nEndY && !bFoundEnough; y++)
		{
			CAliveObject* pObj = pMon->GetMap()->FindTarget(pMon, x, y);
			if (pObj)
			{
				const e_object_type objType = pObj->GetType();
				if (objType == OBJ_PLAYER ||
					(objType == OBJ_PET && ((CMonsterEx*)pObj)->GetDesc()->petset.Type == APT_CALL))
				{
					const int targetX = pObj->getX();
					const int targetY = pObj->getY();
					// 先检查距离（简单的减法）, 距离不符合则跳过方向计算
					const int dx = abs(myX - targetX);
					const int dy = abs(myY - targetY);
					const int maxDiff = dx > dy ? dx : dy;
					if (maxDiff <= attackDist)
					{
						// 判断是否在扇形范围内(前方90度)
						const int objDir = GetDirectionTo(myX, myY, targetX, targetY);
						int objDirDiff = abs(targetDir - objDir);
						if (objDirDiff > 4) objDirDiff = 8 - objDirDiff;
						if (objDirDiff <= 2)
						{
							validTargets.push_back(pObj);
							if (validTargets.size() >= 2)//搜搜2个目标
								bFoundEnough = true;
						}
					}
				}
			}
		}
	}
	//发起攻击
	for (size_t i = 0; i < validTargets.size(); i++)
	{
		CAliveObject* pAttackTarget = validTargets[i];
		WORD wTargetX, wTargetY;
		pAttackTarget->GetPosition(wTargetX, wTargetY);
		pAttackTarget->TakeDamage(pMon->GetId(), nDamage, damagetype, 0,
			DISTANCE(myX, myY, wTargetX, wTargetY) * 50 + 300);

		ThrowDesc desc{};
		desc.x = wTargetX;
		desc.y = wTargetY;
		desc.dwTargetId = pAttackTarget->GetId();
		pMon->SendAroundMsg(pMon->GetId(), 0x16, myX, myY, targetDir, &desc, sizeof(desc));//眨眼睛
		pMon->SendAroundMsg(pMon->GetId(), 0x2bdc, myX, myY, 1, &desc.dwTargetId, sizeof(DWORD));//魔眼魔法动画
	}
}

VOID CMonsterAttack::BloodKing(int nDamage)
{
	if (!pMon || !pMon->GetMap())return;
	const int myX = pMon->getX();
	const int myY = pMon->getY();
	const int targetDir = pMon->GetDirection();
	// 获取当前目标
	CAliveObject* pTarget = pMon->GetTarget();
	if (!pTarget) // 如果没有目标, 寻找最近的玩家
		pTarget = pMon->FindNearestObj(4, OBJ_PLAYER);
	// 随机选择1-6技能
	int nSkillType = Getrand(6);
	switch (nSkillType)
	{
	case 0: // 狂潮飞刺 - 普通攻击, 身前两格敌人重创
		KuangChaoFeiCi(myX, myY, targetDir, nDamage);
		break;
	case 1: // 召唤魔炎 - 魔法攻击, 地面召唤蓝色魔炎, 四格敌人重创
		ZhaoHuanMoYan(myX, myY, nDamage / 2);
		break;
	case 2: // 横扫千军 - 物理攻击, 身前半圆范围重创
		HengSaoQianJun(pTarget, myX, myY, targetDir, nDamage / 2);
		break;
	case 3: // 退敌魔咒 - 魔法战术, 将敌人传送到随机位置
	{
		if (pTarget)
			TuiDiMoZhou(pTarget);
		else// 没有目标则使用普通攻击
			KuangChaoFeiCi(myX, myY, targetDir, nDamage / 2);
	}
	break;
	case 4: // 怒震山河 - 魔法战术, 四格敌人重创并眩晕2秒
	{
		// 检查是否与最后攻击的人距离是否小于等于1
		BOOL boAttack = FALSE;
		CAliveObject* pHitter = pMon->GetHitter();
		if (pHitter)
		{
			if (DISTANCE(pMon->getX(), pMon->getY(), pHitter->getX(), pHitter->getY()) <= 1)
				boAttack = TRUE;
		}
		if (boAttack)
			NuZhenShanHe(pTarget, myX, myY, targetDir, nDamage / 2);
		else
			KuangChaoFeiCi(myX, myY, targetDir, nDamage / 2);
	}
	break;
	case 5: // 移形换位 - 魔法战术, 濒死时瞬移逃生
	{
		// 检查是否濒死（生命值低于30%）
		if (pMon->GetPropValue(PI_CURHP) > 0 && pMon->GetPropValue(PI_CURHP) < pMon->GetPropValue(PI_MAXHP) * 0.3f)
			YiXingHuanWei(targetDir);
		else// 如果不濒死, 使用普通攻击
			KuangChaoFeiCi(myX, myY, targetDir, nDamage / 2);
	}
	break;
	}
}

VOID CMonsterAttack::KuangChaoFeiCi(int myX, int myY, int nDir, int nDamage)
{
	CLogicMap* pMap = pMon->GetMap();
	if (pMap)
	{
		CAliveObject* pFrontObject = nullptr;
		CAliveObject* pFrontObject2 = nullptr;
		GETNEXTPOS(myX, myY, nDir);
		pFrontObject = (CAliveObject*)pMap->FindTarget(pMon, myX, myY);
		GETNEXTPOS(myX, myY, nDir);
		pFrontObject2 = (CAliveObject*)pMap->FindTarget(pMon, myX, myY);
		if (pFrontObject)
			pFrontObject->AddProcess(EP_BEATTACKED, nDamage, pMon->GetId(), DT_PHYSICS, 0, 150);
		if (pFrontObject2)
			pFrontObject2->AddProcess(EP_BEATTACKED, nDamage, pMon->GetId(), DT_PHYSICS, 0, 300);
	}
	// 发送攻击动画消息
	WORD wFlag = (0 << 8) | (WORD)nDir; // 这里计算相当于 MAKEWORD(nDir, 0)
	DWORD dwView[2] = { pMon->GetFeather(), pMon->GetStatus() };
	pMon->SendAroundMsg(pMon->GetId(), SM_ATTACK, myX, myY, wFlag, dwView, sizeof(dwView));
	// 新外形发送
	if (pMon->GetDesc()->base.btImage == 0x222)
		pMon->SendSkill(pMon->GetId(), myX, myY, 2874);
}

VOID CMonsterAttack::ZhaoHuanMoYan(int myX, int myY, int nDamage)
{
	CBlueFireEvent::Create(pMon, myX, myY, nDamage, 2000, (Getrand(5) + 5) * 1000);
}

VOID CMonsterAttack::HengSaoQianJun(CAliveObject* pTarget, int myX, int myY, int nDir, int nDamage)
{
	const int attackRange = 2; // 横扫范围
	// 预计算方向偏移量
	BYTE dirX = G_DIROFS[nDir % 8].x;
	BYTE dirY = G_DIROFS[nDir % 8].y;
	BYTE sideDirX = G_DIROFS[(nDir + 2) % 8].x;
	BYTE sideDirY = G_DIROFS[(nDir + 2) % 8].y;
	for (int depth = 1; depth <= attackRange; depth++)
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
			CAliveObject* pAliveObj = pMon->GetMap()->FindTarget(pMon, sideX, sideY);
			if (pAliveObj)
			{
				const e_object_type objType = pAliveObj->GetType();
				if (objType == OBJ_PLAYER ||
					(objType == OBJ_PET && ((CMonsterEx*)pAliveObj)->GetDesc()->petset.Type == APT_CALL))
				{
					nDamage = nDamage / depth;
					// 发送横扫伤害消息
					pAliveObj->AddProcess(EP_BEATTACKED, nDamage, pMon->GetId(), DT_PHYSICS, 0, depth * 120);
				}
			}
		}
	}
	// 发送攻击动画消息
	WORD wFlag = (2 << 8) | (WORD)nDir;
	DWORD dwView[2] = { pMon->GetFeather(), pMon->GetStatus() };
	pMon->SendAroundMsg(pMon->GetId(), SM_ATTACK, myX, myY, wFlag, dwView, sizeof(dwView));
	// 新外形发送
	if (pMon->GetDesc()->base.btImage == 0x222)
		pMon->SendSkill(pMon->GetId(), myX, myY, 2872);
}

VOID CMonsterAttack::TuiDiMoZhou(CAliveObject* pTarget)
{
	const int myX = pMon->getX();
	const int myY = pMon->getY();
	CLogicMap* pMap = pMon->GetMap();
	if (pMap)
	{
		const int teleportRange = 10; // 传送范围：距离自己10格以内
		const int mapWidth = pMap->GetWidth();
		const int mapHeight = pMap->GetHeight();
		// 尝试20次找到可传送位置
		for (int attempt = 0; attempt < 20; attempt++)
		{
			// 在范围内随机选择角度和距离
			int angle = rand() % 360;
			int distance = 2 + rand() % (teleportRange - 1); // 至少距离2格, 最多10格
			// 计算新坐标
			double radians = angle * 3.14159265358979323846 / 180.0;
			int newX = myX + (int)(cos(radians) * distance);
			int newY = myY + (int)(sin(radians) * distance);
			// 检查位置是否在地图范围内
			if (newX < 0 || newX >= mapWidth || newY < 0 || newY >= mapHeight)
				continue;
			// 检查位置是否可用（IsBlocked返回TRUE表示阻挡）
			if (!pMap->IsBlocked(newX, newY))
			{
				// 传送目标到新位置
				pTarget->FlyTo(pMon->GetMap(), newX, newY);
				break;
			}
		}
	}
}

VOID CMonsterAttack::NuZhenShanHe(CAliveObject* pTarget, int myX, int myY, int nDir, int nDamage)
{
	const int range = VIEW_RANGE / 2;
	for (int x = myX - range; x <= myX + range; x++)
	{
		for (int y = myY - range; y <= myY + range; y++)
		{
			CAliveObject* pObj = pMon->GetMap()->FindTarget(pMon, x, y);
			if (pObj)
			{
				const e_object_type objType = pObj->GetType();
				if (objType == OBJ_PLAYER ||
					(objType == OBJ_PET && ((CMonsterEx*)pObj)->GetDesc()->petset.Type == APT_CALL))
				{
					// 添加眩晕效果2秒
					pObj->SetSystemFlag(SF_SLEEP, TRUE, 0, 2000);
					// 发送怒震山河伤害
					pObj->AddProcess(EP_BEATTACKED, nDamage, pMon->GetId(), DT_MAGIC, 0, 200);
				}
			}
		}
	}
	// 发送攻击动画消息
	WORD wFlag = (4 << 8) | (WORD)nDir;
	DWORD dwView[2] = { pMon->GetFeather(), pMon->GetStatus() };
	pMon->SendAroundMsg(pMon->GetId(), SM_ATTACK, myX, myY, wFlag, dwView, sizeof(dwView));
	// 新外形发送
	if (pMon->GetDesc()->base.btImage == 0x222)
		pMon->SendSkill(pMon->GetId(), myX, myY, 2873);
}

VOID CMonsterAttack::YiXingHuanWei(int nDir)
{
	const int myX = pMon->getX();
	const int myY = pMon->getY();
	CLogicMap* pMap = pMon->GetMap();
	if (pMap)
	{
		const int mapWidth = pMap->GetWidth();
		const int mapHeight = pMap->GetHeight();
		// 尝试10次找到可传送位置
		for (int attempt = 0; attempt < 10; attempt++)
		{
			int newX = myX + Getrand(5) + 2;
			int newY = myY + Getrand(5) + 2;
			// 检查位置是否在地图范围内
			if (newX < 0 || newX >= mapWidth || newY < 0 || newY >= mapHeight)
				continue;
			if (!pMap->IsBlocked(newX, newY))
			{
				pMon->FlyTo(pMon->GetMap(), newX, newY);
				break;
			}
		}
	}
}
