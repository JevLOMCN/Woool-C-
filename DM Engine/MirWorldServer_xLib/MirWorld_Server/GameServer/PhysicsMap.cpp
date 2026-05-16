#include "StdAfx.h"
#include ".\physicsmap.h"
#include "logicmap.h"
#include <stdlib.h>

CPhysicsMap::CPhysicsMap(void)
{
    m_dwVer = 0;
    m_iWidth = 0;
    m_iHeight = 0;
    m_iMaxBlockElements = 0;
    m_pBlockLayer = nullptr;
	m_szMapName[0] = 0;
}

CPhysicsMap::~CPhysicsMap(void)
{
}

BOOL CPhysicsMap::LoadMap(const char* pszFilename)
{
	_splitpath(pszFilename, nullptr, nullptr, m_szMapName, nullptr);
	FILE* fp = fopen(pszFilename, "rb");
	if (fp == nullptr) return FALSE;
	DWORD dwTemp;
	fread(&dwTemp, 4, 1, fp); // 头部数据大小
	fread(&m_dwVer, 4, 1, fp); // 地图类型
	fread(&m_iWidth, 4, 1, fp); // 宽
	fread(&m_iHeight, 4, 1, fp);//高
	//	计算遮挡块
	int f = m_iWidth * m_iHeight;
	m_iMaxBlockElements = (f + 31) / 32;
	m_pBlockLayer = new DWORD[m_iMaxBlockElements];
	memset(m_pBlockLayer, 0, sizeof(DWORD) * m_iMaxBlockElements);

    fseek(fp, 0, SEEK_END);      // 移动到文件末尾
    long nMapSize = ftell(fp) - dwTemp; // 文件大小

	fseek(fp, (long)dwTemp, SEEK_SET); // 跳过头部数据

	const unsigned char g_MapLattice1[32] = { 0x01, 0x01, 0x03, 0x03, 0x03, 0x03, 0x05, 0x05, 0x05, 0x05, 0x07, 0x07, 0x07, 0x07, 0x09, 0x09, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x06, 0x06, 0x06, 0x06, 0x08, 0x08, 0x08, 0x08, 0x0A, 0x0A };
	const unsigned char g_MapLattice2[32] = { 0x01, 0x01, 0x03, 0x03, 0x03, 0x03, 0x05, 0x05, 0x07, 0x07, 0x09, 0x09, 0x09, 0x09, 0x0B, 0x0B, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x06, 0x06, 0x08, 0x08, 0x0A, 0x0A, 0x0A, 0x0A, 0x0C, 0x0C };
	const unsigned char g_MapLattice3[32] = { 0x03, 0x03, 0x05, 0x05, 0x05, 0x05, 0x07, 0x07, 0x09, 0x09, 0x0B, 0x0B, 0x0B, 0x0B, 0x0D, 0x0D, 0x04, 0x04, 0x06, 0x06, 0x06, 0x06, 0x08, 0x08, 0x0A, 0x0A, 0x0C, 0x0C, 0x0C, 0x0C, 0x0E, 0x0E };
	const unsigned char g_MapLattice4[32] = { 0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x0B, 0x0B, 0x09, 0x09, 0x0D, 0x0D, 0x0D, 0x0D, 0x11, 0x11, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08, 0x0C, 0x0C, 0x0A, 0x0A, 0x0E, 0x0E, 0x0E, 0x0E, 0x12, 0x12 };

    // TG地图格子结构
    struct WolMapLattice
    {
        int nWidth;
        int nHeight;
        int nMapFlag;
    };

	BYTE flag;
	DWORD elemindex = 0, bitindex = 0;
	while (f--)
	{   // 根据地图类型读取不同的数据
        if (m_dwVer == 1)// TG地图
        {
            WolMapLattice lattice;
            fread(&lattice, sizeof(WolMapLattice), 1, fp);
            BYTE chFlag = (lattice.nMapFlag > 1000) ? 0 : 1;
            if (chFlag) // 设置遮挡块
                m_pBlockLayer[elemindex] |= (1 << bitindex);
        }
        else
        {
            fread(&flag, 1, 1, fp);
            BYTE nMapLattice = flag;
            BYTE chFlag = (nMapLattice & 0x1);
            if (chFlag) // 设置遮挡块
                m_pBlockLayer[elemindex] |= (1 << bitindex);
            switch (m_dwVer)
            {
            case 3: //普通地图
            {
                BYTE nMapLattice = flag & 0x1F;
                if (nMapLattice > 0)
                {
                    if (nMapLattice < sizeof(g_MapLattice1))
                        fseek(fp, g_MapLattice1[nMapLattice] - 1, SEEK_CUR); // 减去已经读取的 1 字节
                }
            }
            break;
            case 5: // 高速地图
            {
                BYTE nMapLattice = flag & 0x1F;
                if (nMapLattice > 0)
                {
                    if (nMapLattice < sizeof(g_MapLattice2))
                        fseek(fp, g_MapLattice2[nMapLattice] - 1, SEEK_CUR); // 减去已经读取的 1 字节
                }
            }
            break;
            case 6: // Woool 1.978
            {
                BYTE nMapLattice = flag & 0x1F;
                if (nMapLattice > 0)
                {
                    if (nMapLattice < sizeof(g_MapLattice3))
                        fseek(fp, g_MapLattice3[nMapLattice] - 1, SEEK_CUR); // 减去已经读取的 1 字节
                }
                else
                    fseek(fp, 2, SEEK_CUR); // 跳过 2 字节
            }
            break;
            case 7: // Woool 1.996
            {
                BYTE nMapLattice = flag & 0x1F;
                BYTE nMapLattice2 = 0;
                // 读取 nMapLattice2
                if (ftell(fp) < nMapSize)
                    fread(&nMapLattice2, 1, 1, fp);
                // 根据 nMapLattice 跳过相应字节
                if (nMapLattice < sizeof(g_MapLattice4))
                {
                    int skipBytes = g_MapLattice4[nMapLattice] - 2; // 减去已经读取的 2 字节
                    if (skipBytes > 0)
                        fseek(fp, skipBytes, SEEK_CUR);
                }
                // 根据 nMapLattice2 处理额外数据
                if (nMapLattice2 > 0)
                {
                    if (nMapLattice2 & 1) fseek(fp, 2, SEEK_CUR);
                    if (nMapLattice2 & 2) fseek(fp, 4, SEEK_CUR);
                    if (nMapLattice2 & 4) fseek(fp, 4, SEEK_CUR);
                    if (nMapLattice2 & 8) fseek(fp, 4, SEEK_CUR);
                    if (nMapLattice2 & 16) fseek(fp, 4, SEEK_CUR);
                }
            }
            break;
            case 9: // Woool 1.998
            {
                unsigned short wMapLattice = 0;
                if (ftell(fp) < nMapSize)
                {
                    fread(&wMapLattice, 2, 1, fp);
                }
                // 处理标志位
                if (flag & 2) fseek(fp, 4, SEEK_CUR);
                if (flag & 4) fseek(fp, 4, SEEK_CUR);
                if (flag & 8) fseek(fp, 4, SEEK_CUR);
                if (flag & 16) fseek(fp, 1, SEEK_CUR);
                if (flag & 32) fseek(fp, 4, SEEK_CUR);
                if (flag & 64) fseek(fp, 4, SEEK_CUR);
                // 处理 wMapLattice
                if (wMapLattice > 0)
                {
                    if (wMapLattice & 1) fseek(fp, 2, SEEK_CUR);
                    if (wMapLattice & 2) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 4) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 8) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 16) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 32) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x2000) fseek(fp, 4, SEEK_CUR);
                }
            }
            break;
            case 10: // Woool 1.999
            {
                unsigned short wMapLattice = 0;
                if (ftell(fp) < nMapSize)
                {
                    fread(&wMapLattice, 2, 1, fp);
                }
                // 处理标志位
                if (flag & 2) fseek(fp, 4, SEEK_CUR);
                if (flag & 4) fseek(fp, 4, SEEK_CUR);
                if (flag & 8) fseek(fp, 4, SEEK_CUR);
                if (flag & 16) fseek(fp, 1, SEEK_CUR);
                if (flag & 32) fseek(fp, 4, SEEK_CUR);
                if (flag & 64) fseek(fp, 4, SEEK_CUR);
                // 处理 wMapLattice
                if (wMapLattice > 0)
                {
                    if (wMapLattice & 1) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 2) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 4) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 8) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 16) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 32) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x2000) fseek(fp, 4, SEEK_CUR);
                }
            }
            break;
            case 0xB:
            {
                unsigned short wMapLattice = 0;
                // 读取 nMapLattice2 和 wMapLattice
                if (ftell(fp) < nMapSize)
                {
                    fread(&wMapLattice, 2, 1, fp);
                }
                // 处理标志位
                if (flag & 2) fseek(fp, 4, SEEK_CUR);
                if (flag & 4) fseek(fp, 4, SEEK_CUR);
                if (flag & 8) fseek(fp, 4, SEEK_CUR);
                if (flag & 0x10) fseek(fp, 1, SEEK_CUR);
                if (flag & 0x20) fseek(fp, 4, SEEK_CUR);
                if (flag & 0x40) fseek(fp, 4, SEEK_CUR);
                if (wMapLattice > 0)
                {
                    // 处理 wMapLattice
                    if (wMapLattice & 1) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 2) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 4) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 8) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x10) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x20) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x80) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x2000) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x4000) fseek(fp, 2, SEEK_CUR);
                }
            }
            break;
            case 0xC:
            {
                unsigned short wMapLattice = 0;
                if (ftell(fp) < nMapSize)
                {
                    fread(&wMapLattice, 2, 1, fp);
                }
                // 处理标志位
                if (flag & 2) fseek(fp, 4, SEEK_CUR);
                if (flag & 4) fseek(fp, 4, SEEK_CUR);
                if (flag & 8) fseek(fp, 4, SEEK_CUR);
                if (flag & 0x10) fseek(fp, 1, SEEK_CUR);
                if (flag & 0x20) fseek(fp, 4, SEEK_CUR);
                if (flag & 0x40) fseek(fp, 4, SEEK_CUR);
                //if (flag & 0x80) fseek(fp, 4, SEEK_CUR);
                if (wMapLattice > 0)
                {
                    // 处理 wMapLattice
                    if (wMapLattice & 1) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 2) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 4) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 8) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x10) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x20) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x80) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x2000) fseek(fp, 4, SEEK_CUR);
                    if (wMapLattice & 0x4000) fseek(fp, 6, SEEK_CUR);
                }
            }
            break;
            default:
            {
                fclose(fp);
                return FALSE;
            }
            }
        }

		bitindex++;
		if (bitindex >= 32)
		{
			bitindex = 0, elemindex++;
			if (elemindex >= m_iMaxBlockElements)
				break;
		}
	}
	fclose(fp);
	q_strupper(m_szMapName);
	return TRUE;
}

BOOL CPhysicsMap::LoadCache(const char* pszCacheFilename)
{
	_splitpath(pszCacheFilename, nullptr, nullptr, m_szMapName, nullptr);
	FILE* fp = fopen(pszCacheFilename, "rb");
	if (fp == nullptr)return FALSE;
	DWORD dwTemp = 0;
	fread(&dwTemp, 4, 1, fp);
	if (dwTemp != *(DWORD*)"2026")return FALSE;
	fread(&this->m_iWidth, sizeof(int), 1, fp);
	fread(&this->m_iHeight, sizeof(int), 1, fp);
	fread(&this->m_iMaxBlockElements, sizeof(int), 1, fp);
	int f = (m_iWidth * m_iHeight + 31) / 32;
	if (m_iMaxBlockElements > f)m_iMaxBlockElements = f;
	m_pBlockLayer = new DWORD[m_iMaxBlockElements];
    memset(m_pBlockLayer, 0, sizeof(DWORD) * m_iMaxBlockElements);
	fread(m_pBlockLayer, sizeof(DWORD), m_iMaxBlockElements, fp);
	fclose(fp);
	q_strupper(m_szMapName);
	return TRUE;
}

BOOL CPhysicsMap::SaveCache(const char* pszPath)
{
	char szFilename[256];
	_makepath(szFilename, nullptr, pszPath, m_szMapName, ".map");
	FILE* fp = fopen(szFilename, "wb");
	if (fp == nullptr)return FALSE;
	fwrite((void*)"2026", 4, 1, fp);
	fwrite(&m_iWidth, sizeof(int), 1, fp);
	fwrite(&m_iHeight, sizeof(int), 1, fp);
	fwrite(&m_iMaxBlockElements, sizeof(int), 1, fp);
	int f = (m_iWidth * m_iHeight + 31) / 32;
	if (m_iMaxBlockElements > f)f = m_iMaxBlockElements;
	fwrite((void*)m_pBlockLayer, sizeof(DWORD), f, fp);
	fclose(fp);
	return TRUE;
}

BOOL CPhysicsMap::IsBlocked(int x, int y)
{
	if (!VerifyPos(x, y))return TRUE; //坐标非法, 返回阻挡
	if (m_pBlockLayer == nullptr)return FALSE; //阻挡信息为空, 返回无阻挡
	int f = y * m_iWidth + x;
	x = f / 32;
	y = f % 32;
	return ((m_pBlockLayer[x] & (1 << y)) != 0);
}
