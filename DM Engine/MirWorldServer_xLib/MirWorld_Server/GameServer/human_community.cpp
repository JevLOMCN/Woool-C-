#include "StdAfx.h"
#include "humanplayer.h"
#include "humanplayermgr.h"
#include "groupobject.h"
#include "server.h"
#include "systemscript.h"
#include "gameworld.h"

BOOL CHumanPlayer::Marry(CHumanPlayer* pEachOther)
{
	if (pEachOther == this)return FALSE;
	if (pEachOther->IsMarried())return FALSE;
	if (IsMarried())return FALSE;
	pEachOther->SetWife(this);
	this->SetWife(pEachOther);
	return TRUE;
}

BOOL CHumanPlayer::UnMarry()
{
	if (!IsMarried())return FALSE;
	CHumanPlayer* pWife = CHumanPlayerMgr::GetInstance()->FindbyName(m_sWife);
	if (pWife == nullptr)
	{
		//	send unmarry to db
		CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
		if (pObj)
			pObj->SendBreakMarriage(m_sWife, this->GetName());
	}
	else
	{
		//SetWife( nullptr );
		if (strcmp(pWife->m_sWife, this->GetName()) != 0)
			return FALSE;
		pWife->SetWife(nullptr);
	}
	SetWife(nullptr);
	return TRUE;
}

UINT CHumanPlayer::GetStudentCount()const
{
	return (m_sStudents[0][0] != 0) + (m_sStudents[1][0] != 0) + (m_sStudents[2][0] != 0);
}

BOOL CHumanPlayer::IsMyStudent(const char* pszName)const
{
	for (UINT n = 0; n < 3; n++)
	{
		if (strcmp(this->m_sStudents[n], pszName) == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::AddStudent(CHumanPlayer* pStudent)
{
	if (pStudent == this)return FALSE;
	if (IsMyStudent(pStudent->GetName()))
		return FALSE;
	if (pStudent->HasMaster())
		return FALSE;
	for (int i = 0; i < 3; i++)
	{
		if (m_sStudents[i][0] == 0)
		{
			o_strncpy(m_sStudents[i], pStudent->GetName(), 16);
			SendAddCommunity(2, pStudent->GetName());
			pStudent->SetMaster(this);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CHumanPlayer::DeleteStudent(const char* pszName)
{
	CHumanPlayer* pStudent = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
	for (UINT n = 0; n < 3; n++)
	{
		if (strcmp(this->m_sStudents[n], pszName) == 0)
		{
			m_sStudents[n][0] = 0;
			if (pStudent)
				pStudent->SetMaster(nullptr);
			else
			{
				CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
				if (pObj)
					pObj->SendBreakTeacher(this->GetName(), pszName);
			}
			SendDeleteCommunity(2, pszName);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CHumanPlayer::LeaveTeacher()
{
	if (m_sMaster[0] == 0)return FALSE;
	CHumanPlayer* pMaster = CHumanPlayerMgr::GetInstance()->FindbyName(m_sMaster);
	if (pMaster)
	{
		pMaster->DeleteStudent(this->GetName());
		if (m_sMaster[0] != 0)
			SetMaster(nullptr);
	}
	else
	{
		CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
		if (pObj)
			pObj->SendBreakTeacher(m_sMaster, this->GetName());
		SetMaster(nullptr);
	}
	return TRUE;
}

VOID CHumanPlayer::SetMaster(CHumanPlayer* pMaster)
{
	if (pMaster == this) return;
	if (pMaster == nullptr)
	{
		SendDeleteCommunity(1, m_sMaster);
		m_sMaster[0] = 0;
	}
	else
	{
		o_strncpy(m_sMaster, pMaster->GetName(), 16);
		SendAddCommunity(1, m_sMaster);
	}
	UpdateViewName();
}

VOID CHumanPlayer::SetWife(CHumanPlayer* pWife)
{
	if (pWife == this)return;
	if (pWife == nullptr)
	{
		SendDeleteCommunity(3, m_sWife);
		m_sWife[0] = 0;
	}
	else
	{
		o_strncpy(m_sWife, pWife->GetName(), 16);
		SendAddCommunity(3, m_sWife);
	}
	UpdateViewName();
}

static thread_local char g_szTempBuffer[65536];
VOID CHumanPlayer::OnCommunityInfo(const char* pszCommunityInfo)
{
	o_strncpy(g_szTempBuffer, pszCommunityInfo, 65535);
	xStringsExpander<200> community(g_szTempBuffer, '/');
	char* pszWife = community[0] == nullptr ? "" : community[0];
	char* pszMaster = community[1] == nullptr ? "" : community[1];

	o_strncpy(m_sWife, pszWife, 16);
	o_strncpy(m_sMaster, pszMaster, 16);

	int count = 0;
	memset(m_sStudents, 0, sizeof(m_sStudents));
	char* pszStudent = nullptr;
	for (int i = 0; i < 3; i++)
	{
		pszStudent = community[2 + i] == nullptr ? "" : community[2 + i];
		if (pszStudent[0] != 0)
			o_strncpy(m_sStudents[count++], pszStudent, 16);
	}
	count = 0;
	memset(m_sFriends, 0, sizeof(m_sFriends));
	char* pszFriend = nullptr;
	for (int i = 0; i < 32; i++)
	{
		pszFriend = community[5 + i] == nullptr ? "" : community[5 + i];
		if (pszFriend[0] != 0)
			o_strncpy(m_sFriends[count++], pszFriend, 16);
	}
	UpdateCommunityInfoToClient();
	SetSystemFlag(SF_COMMUNITYLOADED, TRUE);
}

int	CHumanPlayer::GetCommunityInfo(char* pszCommunityBuffer, int iSize)
{
	xPacket packet(pszCommunityBuffer, iSize);
	packet.push((LPVOID)GetName(), (int)strlen(GetName()));
	packet.push((LPVOID)"/", 1);
	if (m_sWife[0] != 0)packet.push((LPVOID)m_sWife, (int)strlen(m_sWife));
	packet.push((LPVOID)"/", 1);
	if (m_sMaster[0] != 0)packet.push((LPVOID)m_sMaster, (int)strlen(m_sMaster));
	packet.push((LPVOID)"/", 1);
	for (int i = 0; i < 3; i++)
	{
		if (m_sStudents[i][0] != 0)packet.push((LPVOID)m_sStudents[i], (int)strlen(m_sStudents[i]));
		packet.push((LPVOID)"/", 1);
	}
	for (int i = 0; i < 32; i++)
	{
		if (m_sFriends[i][0] != 0)packet.push((LPVOID)m_sFriends[i], (int)strlen(m_sFriends[i]));
		packet.push((LPVOID)"/", 1);
	}
	return packet.getsize();
}

static thread_local CHAR g_szCodedMsgBuffer[65536];
VOID CHumanPlayer::UpdateCommunityInfoToClient(BOOL bNoticeOnline)
{
	xPacket packet(g_szTempBuffer, 65536);
	xPacket lpacket(g_szCodedMsgBuffer, 65536);
	char szNiticeBuffer[128]{};
	xPacket nPacket(szNiticeBuffer, 128);
	CHumanPlayerMgr* pMgr = CHumanPlayerMgr::GetInstance();
	CHumanPlayer* p = nullptr;
	DWORD flag = 1;
	if (!bNoticeOnline)flag = 3;
	char* pName = (char*)GetName();
	for (int i = 0; i < 32; i++)
	{
		if (m_sFriends[i][0] == 0)continue;
		packet.push((LPVOID)m_sFriends[i], (int)strlen(m_sFriends[i]));
		packet.push((LPVOID)"/", 1);
		if (pMgr && (p = pMgr->FindbyName(m_sFriends[i])))
		{
			nPacket.clear();
			nPacket.push(pName);
			nPacket.push(1);
			nPacket.push(GetGuildName());
			if (bNoticeOnline)p->SendMsg(1, 0x1c5, 0, 0, MAKEWORD(GetPropValue(PI_LEVEL), GetPro()), (LPVOID)nPacket.getbuf(), nPacket.getsize());//在好友那边告诉我在线通知
			nPacket.clear();
			nPacket.push(m_sFriends[i]);
			nPacket.push(1);
			nPacket.push(p->GetGuildName());
			char szTempBuffer[1024];
			int tempSize = 0;
			SmartEncodeMessage(szTempBuffer, tempSize, flag, 0x1c5, 0, 0, MAKEWORD(p->GetPropValue(PI_LEVEL),p->GetPro()), (LPVOID)nPacket.getbuf(), nPacket.getsize());
			memcpy((char*)lpacket.getbuf() + lpacket.getsize(), szTempBuffer, tempSize);
			lpacket.addsize(tempSize);
		}
	}
	if (packet.getsize() > 0)
	{
		//p1关系类型 0好友、1师傅、2徒弟、3配偶、4宗派; p3改名标志(1=改名操作,0=列表操作)
		SendMsg(0, 0x1c1, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize()); // 返回好友列表
		packet.clear();
	}
	if (m_sMaster[0] != 0)
	{
		int i = 0;
		packet.push((LPVOID)m_sMaster, (int)strlen(m_sMaster));
		packet.push((LPVOID)"/", 1);
		SendMsg(0, 0x1c1, 1, 0, 0, (LPVOID)packet.getbuf(), packet.getsize()); // 返回师父列表
		packet.clear();
		if (pMgr && (p = pMgr->FindbyName(m_sMaster)))
		{
			nPacket.clear();
			nPacket.push(pName);
			nPacket.push(1);
			nPacket.push(GetGuildName());
			if (bNoticeOnline)p->SendMsg(1, 0x1c5, 1, 0, MAKEWORD(GetPropValue(PI_LEVEL), GetPro()), (LPVOID)nPacket.getbuf(), nPacket.getsize());
			nPacket.clear();
			nPacket.push(m_sMaster);
			nPacket.push(1);
			nPacket.push(p->GetGuildName());
			char szTempBuffer[1024];
			int tempSize = 0;
			SmartEncodeMessage(szTempBuffer, tempSize, flag, 0x1c5, 1, 0, MAKEWORD(p->GetPropValue(PI_LEVEL), p->GetPro()), (LPVOID)nPacket.getbuf(), nPacket.getsize());
			memcpy((char*)lpacket.getbuf() + lpacket.getsize(), szTempBuffer, tempSize);
			lpacket.addsize(tempSize);
		}
	}
	for (int i = 0; i < 3; i++)
	{
		if (m_sStudents[i][0] == 0)continue;
		packet.push((LPVOID)m_sStudents[i], (int)strlen(m_sStudents[i]));
		packet.push((LPVOID)"/", 1);
		if (pMgr && (p = pMgr->FindbyName(m_sStudents[i])))
		{
			nPacket.clear();
			nPacket.push(pName);
			nPacket.push(1);
			nPacket.push(GetGuildName());
			if (bNoticeOnline)p->SendMsg(1, 0x1c5, 2, 0, MAKEWORD(GetPropValue(PI_LEVEL), GetPro()), (LPVOID)nPacket.getbuf(), nPacket.getsize());
			nPacket.clear();
			nPacket.push(m_sStudents[i]);
			nPacket.push(1);
			nPacket.push(p->GetGuildName());
			char szTempBuffer[1024];
			int tempSize = 0;
			SmartEncodeMessage(szTempBuffer, tempSize, flag, 0x1c5, 2, 0, MAKEWORD(p->GetPropValue(PI_LEVEL), p->GetPro()), (LPVOID)nPacket.getbuf(), nPacket.getsize());
			memcpy((char*)lpacket.getbuf() + lpacket.getsize(), szTempBuffer, tempSize);
			lpacket.addsize(tempSize);
		}
	}
	if (packet.getsize() > 0)
	{
		SendMsg(0, 0x1c1, 2, 0, 0, (LPVOID)packet.getbuf(), packet.getsize()); // 返回徒弟列表
		packet.clear();
	}
	if (m_sWife[0] != 0)
	{
		int i = 0;
		packet.push((LPVOID)m_sWife, (int)strlen(m_sWife));
		packet.push((LPVOID)"/", 1);
		SendMsg(0, 0x1c1, 3, 0, 0, (LPVOID)packet.getbuf(), packet.getsize()); // 返回夫妻列表
		if (pMgr && (p = pMgr->FindbyName(m_sWife)))
		{
			nPacket.clear();
			nPacket.push(pName);
			nPacket.push(1);
			nPacket.push(GetGuildName());
			if (bNoticeOnline)p->SendMsg(1, 0x1c5, 3, 0, MAKEWORD(GetPropValue(PI_LEVEL), GetPro()), (LPVOID)nPacket.getbuf(), nPacket.getsize());
			nPacket.clear();
			nPacket.push(m_sWife);
			nPacket.push(1);
			nPacket.push(p->GetGuildName());
			char szTempBuffer[1024];
			int tempSize = 0;
			SmartEncodeMessage(szTempBuffer, tempSize, flag, 0x1c5, 3, 0, MAKEWORD(p->GetPropValue(PI_LEVEL), p->GetPro()), (LPVOID)nPacket.getbuf(), nPacket.getsize());
			memcpy((char*)lpacket.getbuf() + lpacket.getsize(), szTempBuffer, tempSize);
			lpacket.addsize(tempSize);
		}
	}

	if (m_sWife[0] != 0 || m_sMaster[0] != 0)
		this->UpdateViewName();
	OnAroundMsg(nullptr, lpacket.getbuf(), lpacket.getsize());
}

VOID CHumanPlayer::SendAddFriend(const char* pszName)
{
	SendMsg(0, 0x1c2, 0, 0, 0, (LPVOID)pszName);//增加了一个好友
}

VOID CHumanPlayer::SendDelFriend(const char* pszName)
{
	SendMsg(0, 0x1c3, 0, 0, 0, (LPVOID)pszName);//删除了一个好友
}

BOOL CHumanPlayer::AddFriend(CHumanPlayer* pFriend)
{
	int freeptr = -1;
	int friendfreeptr = -1;
	if (pFriend == nullptr)return FALSE;
	if (pFriend == this)return FALSE;

	char* pFriendName = (char*)pFriend->GetName();
	char* pName = (char*)GetName();

	for (int i = 0; i < 32; i++)
	{
		if (m_sFriends[i][0] == 0)
			if (freeptr == -1) freeptr = i;
		else
		{
			if (strcmp(m_sFriends[i], pFriendName) == 0)
			{
				SendFriendSystemError(FE_ADD_ALREADYFRIEND, pFriendName);
				pFriend->SendFriendSystemError(FE_ADD_ALREADYFRIEND, pName);
				return FALSE;
			}
		}
		if (pFriend->m_sFriends[i][0] == 0)
			if (friendfreeptr == -1)friendfreeptr = i;
		else
		{
			if (strcmp(pFriend->m_sFriends[i], pName) == 0)
			{
				SendFriendSystemError(FE_ADD_ALREADYFRIEND, pFriendName);
				pFriend->SendFriendSystemError(FE_ADD_ALREADYFRIEND, pName);
				return FALSE;
			}
		}
	}
	if (freeptr == -1)
	{
		pFriend->SendFriendSystemError(FE_ACCEPT_FULL, GetName());
		SendFriendSystemError(FE_ADD_FULL, pFriend->GetName());
		return FALSE;
	}
	if (friendfreeptr == -1)
	{
		SendFriendSystemError(FE_ACCEPT_FULL, pFriend->GetName());
		pFriend->SendFriendSystemError(FE_ADD_FULL, GetName());
		return FALSE;
	}

	o_strncpy(m_sFriends[freeptr], pFriend->GetName(), 16);
	o_strncpy(pFriend->m_sFriends[friendfreeptr], GetName(), 16);

	SendAddFriend(pFriend->GetName());
	pFriend->SendAddFriend(GetName());
	return TRUE;
}

BOOL CHumanPlayer::DeleteFriend(const char* pszName)
{
	for (int i = 0; i < 32; i++)
	{
		if (strcmp(pszName, m_sFriends[i]) == 0)
		{
			m_sFriends[i][0] = 0;
			for (int j = i; j < 31; j++)
			{
				o_strncpy(m_sFriends[j], m_sFriends[j + 1], 16);
			}
			SendDelFriend(pszName);
			CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
			if (pPlayer)
			{
				for (int j = 0; j < 32; j++)
				{
					pszName = GetName();
					if (strcmp(pszName, pPlayer->m_sFriends[j]) == 0)
					{
						pPlayer->m_sFriends[j][0] = 0;
						for (int k = j; k < 31; k++)
						{
							o_strncpy(pPlayer->m_sFriends[k], pPlayer->m_sFriends[k + 1], 16);
						}
						pPlayer->SendDelFriend(GetName());
					}
				}
			}
			else
			{
				CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
				if (pObj)
					pObj->SendBreakFriend(pszName, GetName());
			}
			return TRUE;
		}
	}
	return FALSE;
}


VOID CHumanPlayer::SendFriendSystemError(friend_error fe, const char* pszName)
{
	SendMsg((DWORD)fe, 0x1c4, 0, 0, 0, (LPVOID)pszName);//增加删除的出错信息
}

VOID CHumanPlayer::ToggleFriendMode()
{
	if (this->m_bRefuseAddFriend)
	{
		SaySystemAttrib(CC_GREEN, getstrings(SD_ALLOWFRIENDREQUEST));//"[允许加为好友]" );
		m_bRefuseAddFriend = FALSE;
	}
	else
	{
		SaySystemAttrib(CC_GREEN, getstrings(SD_REFUSEFRIENDREQUEST));//"[拒绝加为好友]" );
		m_bRefuseAddFriend = TRUE;
	}
}

VOID CHumanPlayer::NoticeFriendOffline()
{
	CHumanPlayerMgr* pMgr = CHumanPlayerMgr::GetInstance();
	CHumanPlayer* p = nullptr;
	char* pName = (char*)this->GetName();
	for (int i = 0; i < 32; i++)
	{
		if (m_sFriends[i][0] == 0)continue;
		if (pMgr && (p = pMgr->FindbyName(m_sFriends[i])))
			p->SendMsg(0, 0x1c5, 0, 0, GetPropValue(PI_LEVEL), (LPVOID)pName);
	}
	for (int i = 0; i < 3; i++)
	{
		if (m_sStudents[i][0] == 0)continue;
		if (pMgr && (p = pMgr->FindbyName(m_sStudents[i])))
			p->SendMsg(0, 0x1c5, 2, 0, GetPropValue(PI_LEVEL), (LPVOID)pName);
	}
	if (m_sMaster[0] != 0)
	{
		if (pMgr && (p = pMgr->FindbyName(m_sMaster)))
			p->SendMsg(0, 0x1c5, 1, 0, GetPropValue(PI_LEVEL), (LPVOID)pName);
	}
	if (m_sWife[0] != 0)
	{
		if (pMgr && (p = pMgr->FindbyName(m_sWife)))
			p->SendMsg(0, 0x1c5, 3, 0, GetPropValue(PI_LEVEL), (LPVOID)pName);
	}
}

#include "guildex.h"
const char* CHumanPlayer::GetGuildName()
{
	if (this->m_pGuild == nullptr)return "";
	return m_pGuild->GetName();
}
