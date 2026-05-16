#include "stdafx.h"
#include "FengHaoGrowManager.h"
#include "FmtTextFile.h"

CFengHaoGrowManager::CFengHaoGrowManager(void)
{
    m_pFengHaoGrows = nullptr;
    m_iFengHaoGrowCount = 0;
}

CFengHaoGrowManager::~CFengHaoGrowManager(void)
{
    if (m_pFengHaoGrows)
    {
        delete[] m_pFengHaoGrows;
        m_pFengHaoGrows = nullptr;
    }
    m_iFengHaoGrowCount = 0;
}

VOID CFengHaoGrowManager::LoadData(const char* pszData, BOOL bCSV)
{
    CFmtTextFile ftf("b1s16b3s64s64s64s64b1", pszData, bCSV);
    if (ftf.GetCount() == 0) return;
    if (m_pFengHaoGrows) delete[] m_pFengHaoGrows;
    m_pFengHaoGrows = new FengHaoGrowItem[ftf.GetCount()];
    m_iFengHaoGrowCount = 0;
    TempItem temp{};
    for (int i = 0; i < ftf.GetCount(); i++)
    {
        if (ftf.GetStruct(i, &temp))
        {
            FengHaoGrowItem& item = m_pFengHaoGrows[m_iFengHaoGrowCount];
            item.btId = temp.id;
            memcpy(item.szName, temp.name, 16);
            item.btType = temp.type;
            item.btLastDay = temp.lastDay;
            item.nAttrCnt = temp.attrCnt;
            item.btColorId = temp.colorId;
            for (int j = 0; j < 4; j++)
            {
                sscanf_s(temp.szAttrs[j], "%hhu;%hu;%hu", &item.attrs[j].nAttrType, &item.attrs[j].nValue1, &item.attrs[j].nValue2);
            }
            m_iFengHaoGrowCount++;
        }
    }
}

FengHaoGrowItem* CFengHaoGrowManager::GetItem(BYTE index)
{
    for (int i = 0; i < m_iFengHaoGrowCount; i++)
    {
        if (m_pFengHaoGrows[i].btId == index)
            return &m_pFengHaoGrows[i];
    }
    return nullptr;
}