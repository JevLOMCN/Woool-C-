#pragma once
#include "stdafx.h"
#include "MonsterEx.h"

/**
 * @brief 怪死亡事件
 */
class MonsterDeathEvent : public BaseEvent
{
public:
    CMonsterEx* monster;           //掉落物品的怪物
    CHumanPlayer* killer;          //击杀者
    BOOL m_boDropItem;             //是否掉落物品
    MonsterDeathEvent(CMonsterEx* pMonster, CHumanPlayer* pKiller, BOOL boDropItem) : monster(pMonster), killer(pKiller)
    {
        m_boDropItem = boDropItem;
    }
};
