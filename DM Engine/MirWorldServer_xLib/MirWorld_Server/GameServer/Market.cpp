#include "StdAfx.h"
#include ".\market.h"
#include ".\submarket.h"
#include ".\humanplayer.h"

CMarket::CMarket(UINT id) : m_pSubMarketArray{}
{
	m_nSubMarketCount = 0;
	this->m_Id = id;
}

CMarket::~CMarket(void)
{
	for (UINT n = 0; n < this->m_nSubMarketCount; n++)
	{
		if (this->m_pSubMarketArray[n])
			delete this->m_pSubMarketArray[n];
	}
}

CSubMarket* CMarket::AddSubMarket(UINT nId, const char* pszName)
{
	if (this->m_nSubMarketCount >= 32)
		return nullptr;
	char szFilename[256];

	this->m_pSubMarketArray[this->m_nSubMarketCount] = new CSubMarket(this->m_Id, nId, pszName);

	sprintf(szFilename, ".\\Data\\Market\\%02u%02u.txt", this->m_Id, nId);
	if (!this->m_pSubMarketArray[this->m_nSubMarketCount]->LoadSubMarket(szFilename))
	{
		delete this->m_pSubMarketArray[this->m_nSubMarketCount];
		return nullptr;
	}
	return this->m_pSubMarketArray[this->m_nSubMarketCount++];
}

CSubMarket* CMarket::GetSubMarket(UINT nId)
{
	for (UINT n = 0; n < this->m_nSubMarketCount; n++)
	{
		if (this->m_pSubMarketArray[n] && this->m_pSubMarketArray[n]->GetId() == nId)
			return this->m_pSubMarketArray[n];
	}
	return nullptr;
}

static thread_local char g_szTempString[65536];
VOID CMarket::SendSubMarket(CHumanPlayer* pPlayer)
{
	xPacket packet(g_szTempString, 65536);
	sprintf((char*)packet.getbuf(), "%02u&", m_Id);
	packet.addsize((int)strlen(packet.getbuf()));
	for (UINT n = 0; n < this->m_nSubMarketCount; n++)
	{
		if (this->m_pSubMarketArray[n])
		{
			sprintf((char*)packet.getfreebuf(), "%02u|%s&", this->m_pSubMarketArray[n]->GetId(), this->m_pSubMarketArray[n]->GetName());
			packet.addsize((int)strlen(packet.getfreebuf()));
		}
	}
	pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 2, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	if (this->m_nSubMarketCount > 0 && m_pSubMarketArray[0])
		m_pSubMarketArray[0]->QueryItems(pPlayer, GetId());
}