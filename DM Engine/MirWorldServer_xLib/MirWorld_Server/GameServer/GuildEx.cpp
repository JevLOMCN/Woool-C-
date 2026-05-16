#include "StdAfx.h"
#include "server.h"
#include ".\guildex.h"
#include "humanplayer.h"
#include "gameworld.h"

enum e_readguild
{
	RG_NONE,
	RG_NOTICE, // 公告
	RG_MEMBERS, // 行会成员
	RG_KILLGUILDS, // 敌对行会
	RG_ALLYGUILDS, // 联盟行会
	RG_DECLARATION, // 行会宣言
	RG_APPLYLIST, //申请列表
};
CGuildMemberEx::CGuildMemberEx()
{
	m_szCharName[0] = 0;
	m_dwInstanceKey = 0;
	m_pNode = nullptr;
	m_pRefPlayer = nullptr;
	m_btJob = 0;
	m_wLevel = 0;
	m_nExp = 0;
	m_nPower = 0;
	m_pGroup = nullptr;
	m_boNoSay = FALSE;
}

CGuildMemberEx::~CGuildMemberEx()
{
}

VOID CGuildMemberEx::Init(const char* pszName, UINT nExp, UINT nPower, WORD wLevel, BYTE btJob, CHumanPlayer* pRefPlayer)
{
	o_strncpy(m_szCharName, pszName, 16);
	m_nExp = nExp;
	m_nPower = nPower;
	m_btJob = btJob;
	m_wLevel = wLevel;
	SetRefPlayer(pRefPlayer);
}

BOOL CGuildMemberEx::IsRefValid()
{
	if (m_pRefPlayer)
	{
		if (m_dwInstanceKey == m_pRefPlayer->GetInstanceKey())
			return TRUE;
		m_pRefPlayer = nullptr;
	}
	return FALSE;
}

VOID CGuildMemberEx::SetRefPlayer(CHumanPlayer* pPlayer)
{
	m_pRefPlayer = pPlayer;
	if (m_pRefPlayer)
		m_dwInstanceKey = m_pRefPlayer->GetInstanceKey();
	else
		m_dwInstanceKey = 0;
}


xObjectPool<GUILDMEMBERNODE> CGuildGroupEx::m_xGuildMemberNodePool;
CGuildGroupEx::CGuildGroupEx()
{
	m_nLevel = 0;
	m_szName[0] = 0;
}

CGuildGroupEx::~CGuildGroupEx()
{
}

BOOL CGuildGroupEx::AddMember(CGuildMemberEx* pMember, BOOL bConfirm)
{
	GUILDMEMBERNODE* pNode = m_xGuildMemberNodePool.newObject();
	if (pNode == nullptr)return FALSE;
	pNode->setObject(pMember);
	if (!m_xMemberList.addNode(pNode))return FALSE;
	if (bConfirm)
	{
		if (pMember->getNode() != pNode)
		{
			if (pMember->getNode())
			{
				pMember->getNode()->Leave();
				m_xGuildMemberNodePool.deleteObject(pMember->getNode());
			}
			pMember->setNode(pNode);

			if (pMember->GetGroup() != this)
				pMember->SetGroup(this);
		}
	}
	return TRUE;
}

VOID CGuildGroupEx::RemoveMember(CGuildMemberEx* pMember)
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	while (pNode)
	{
		if (pNode->getObject() == pMember)
		{
			m_xMemberList.removeNode(pNode);
			if (pMember->getNode() == pNode)
				pMember->setNode(nullptr);
			if (pMember->GetGroup() == this)
				pMember->SetGroup(nullptr);
			m_xGuildMemberNodePool.deleteObject(pNode);
			return;
		}
		pNode = pNode->getNext();
	}
}

VOID CGuildGroupEx::ConfirmMembers()
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	while (pNode)
	{
		if (pNode->getObject()->getNode() != pNode)
		{
			if (pNode->getObject()->getNode())
			{
				pNode->getObject()->getNode()->Leave();
				m_xGuildMemberNodePool.deleteObject(pNode->getObject()->getNode());
			}
			pNode->getObject()->setNode(pNode);

			if (pNode->getObject()->GetGroup() != this)
				pNode->getObject()->SetGroup(this);
		}
		pNode = pNode->getNext();
	}
}

VOID CGuildGroupEx::ClearAllRef()
{
	GUILDMEMBERNODE* pNode = nullptr;
	while (pNode = m_xMemberList.getHead())
	{
		if (pNode->getObject() && pNode->getObject()->getNode() == pNode)
			pNode->getObject()->setNode(nullptr);
		pNode->setObject(nullptr);
		m_xMemberList.removeNode(pNode);
		m_xGuildMemberNodePool.deleteObject(pNode);
	}
}

static thread_local char g_szTempBuffer[65536];
VOID CGuildGroupEx::SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData, int iDataSize)
{
	int length = EncodeMsg(g_szTempBuffer, dwFlag, wCmd, w1, w2, w3, lpData, iDataSize);
	if (length > 0)
		OnMsg(g_szTempBuffer, length);
}

VOID CGuildGroupEx::OnMsg(const char* pszMsg, int iMsgLength)
{
	auto* pNode = m_xMemberList.getHead();
	while (pNode)
	{
		CHumanPlayer* pRefPlayer = pNode->getObject()->GetRefPlayer();
		if (pRefPlayer)
			pRefPlayer->OnAroundMsg(nullptr, pszMsg, iMsgLength);
		pNode = pNode->getNext();
	}
}

VOID CGuildGroupEx::SaveToFile(FILE* fp)
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	if (pNode == nullptr)return;
	while (pNode->getNext())pNode = pNode->getNext();
	while (pNode)
	{
		fprintf(fp, "+%s|%u|%u|%u|%u\n", pNode->getObject()->GetName(), pNode->getObject()->GetLevel(), 
			pNode->getObject()->GetPro(), pNode->getObject()->GetExp(), pNode->getObject()->GetPower());
		pNode = pNode->getPrev();
	}
}

VOID CGuildGroupEx::SendWords(const char* pszWords)
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	while (pNode)
	{
		if (pNode->getObject()->IsRefValid())
			pNode->getObject()->GetRefPlayer()->ChannelHearDirectly(CCH_GUILD, 0, pszWords);
		pNode = pNode->getNext();
	}
}

VOID CGuildGroupEx::UpdateNames()
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	while (pNode)
	{
		if (pNode->getObject()->IsRefValid())
			pNode->getObject()->GetRefPlayer()->UpdateViewName();
		pNode = pNode->getNext();
	}
}

VOID CGuildGroupEx::AppendDurationMembers(xPacket& packet)
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	if (pNode == nullptr)return;
	while (pNode->getNext())pNode = pNode->getNext();
	while (pNode)
	{
		CHumanPlayer* pPlayer = pNode->getObject()->GetRefPlayer();
		packet.push("0");
		packet.push("0"); // 天人境界类型
		packet.push(pNode->getObject()->GetName());
		packet.push("/");
		pNode = pNode->getPrev();
	}
}

VOID CGuildGroupEx::AppendDurationMembers2(xPacket& packet)
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	if (pNode == nullptr)return;
	while (pNode->getNext())pNode = pNode->getNext();
	while (pNode)
	{
		CHumanPlayer* pPlayer = pNode->getObject()->GetRefPlayer();
		if (pPlayer)
		{
			packet.push(pPlayer->GetName()); //m_xMemberList
			packet.push(1);
			BYTE boOnLine = TRUE;
			packet.push((LPVOID)&boOnLine, 1); // 在线
			packet.push(4); // 本周贡献值
			packet.push(4); // 历史贡献值
			BYTE btPro = pPlayer->GetPro();
			packet.push(&btPro, 1); // 职业
			WORD wLevel = pPlayer->GetPropValue(PI_LEVEL);
			packet.push(&wLevel, 2); // 等级
		}
		else
		{
			packet.push(pNode->getObject()->GetName());
			packet.push(1);
			packet.push(1); // 不在线
			packet.push(4); // 本周贡献值
			packet.push(4); // 历史贡献值
			BYTE btPro = pNode->getObject()->GetPro();
			packet.push(&btPro, 1); // 职业
			WORD wLevel = pNode->getObject()->GetLevel();
			packet.push(&wLevel, 2); // 等级
		}
		pNode = pNode->getPrev();
	}
}

VOID CGuildGroupEx::ReviewAroundNameColor()
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	while (pNode)
	{
		if (pNode->getObject()->IsRefValid())
			pNode->getObject()->GetRefPlayer()->ReviewAroundNameColor();
		pNode = pNode->getNext();
	}
}

CGuildMemberEx* CGuildGroupEx::GetFirstMember()
{
	GUILDMEMBERNODE* pNode = m_xMemberList.getHead();
	if (pNode == nullptr)return nullptr;
	while (pNode->getNext())pNode = pNode->getNext();
	return pNode->getObject();
}


xObjectPool<CGuildMemberEx> CGuildEx::m_xMemberPool;
xObjectPool<CGuildGroupEx> CGuildEx::m_xGroupPool;
CGuildEx::CGuildEx(void)
{
	memset(m_xGroups, 0, sizeof(m_xGroups));
	m_nGroupCount = 0;
	m_szName[0] = 0;
	m_szNotice[0] = 0;
	m_szFilename[0] = 0;
	m_nLevel = 0;
	m_nExp = 0;
	m_nGold = 0;
	memset(m_sAllyGuilds, 0, sizeof(m_sAllyGuilds));
	m_nAllyGuildCount = 0;
	memset(m_sKillGuilds, 0, sizeof(m_sKillGuilds));
	m_nKillGuildCount = 0;
	m_fAttackSabuk = FALSE;
	m_boRecruitState = TRUE;
	m_nMaxMemberCount = 0;
	m_boAllNoSay = FALSE;
}

CGuildEx::~CGuildEx(void)
{
}

CGuildGroupEx* CGuildEx::NewGroup(const char* pszName, UINT nLevel) const
{
	if (m_nGroupCount >= 99) return nullptr;//行会封号组最多支持99组
	if (nLevel == 0) nLevel = m_nGroupCount;
	CGuildGroupEx* pGroup = m_xGroupPool.newObject();
	if (pGroup == nullptr)return nullptr;
	pGroup->SetName(pszName);
	pGroup->SetLevel(nLevel);
	return pGroup;
}

VOID CGuildEx::DeleteGroup(CGuildGroupEx* pGroup)
{
	pGroup->ClearAllRef();
	m_xGroupPool.deleteObject(pGroup);
}

BOOL CGuildEx::AddGroupToList(CGuildGroupEx* pGroup)
{
	int pos = m_nGroupCount;
	for (int i = 0; i < (int)m_nGroupCount; i++)
	{
		if (m_xGroups[i]->GetLevel() < pGroup->GetLevel())
			continue;
		pos = i;
		break;
	}
	memmove((void*)(m_xGroups + pos + 1), (void*)(m_xGroups + pos), sizeof(CGuildGroupEx*) * ((int)m_nGroupCount - pos));
	m_xGroups[pos] = pGroup;
	m_nGroupCount++;
	return TRUE;
}

VOID CGuildEx::DeleteGroupFromList(CGuildGroupEx* pGroup)
{
	BOOL bReload = FALSE;
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (pGroup == m_xGroups[n])
		{
			m_xGroups[n] = nullptr;
			bReload = TRUE;
		}
		if (bReload)
			m_xGroups[n] = m_xGroups[n + 1];
	}
	m_nGroupCount--;
	DeleteGroup(pGroup);
}

BOOL CGuildEx::ChangeGroup(const char* pszName, UINT nLevel, UINT nOldLevel)
{
	CGuildMemberEx* pGuildMember = GetMember(pszName);
	if (pGuildMember == nullptr) return FALSE;
	CGuildGroupEx* pOldGroup = pGuildMember->GetGroup();
	if (pOldGroup && pOldGroup->GetLevel() == nLevel) return FALSE; // 已经在目标分组

	CGuildGroupEx* pNewGroup = GetGroupByLevel(nLevel);
	if (pNewGroup == nullptr) return FALSE;

	if (pOldGroup) pOldGroup->RemoveMember(pGuildMember);
	pNewGroup->AddMember(pGuildMember, TRUE);
	Save();

	char szText[256];
	if (nLevel < 11)
	{
		sprintf(szText, "%s 被任命为 %s!", pszName, pNewGroup->GetName());
		SendWords(szText);
	}
	else
	{
		sprintf(szText, "%s 被免职!", pszName);
		SendWords(szText);
	}
	CHumanPlayer* p = pGuildMember->GetRefPlayer();
	if (p) SendGuildTowerInfo(p);//如果玩家在线，刷新他的权限
	return TRUE;
}

VOID CGuildEx::SendGroupFengHaoData(CHumanPlayer* pPlayer)
{
	xPacket packet(g_szTempBuffer, 65535);
	xPacket packet1(4096);
	packet.push(&m_nGroupCount, 4);
	packet1.push(&m_nGroupCount, 4);
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (m_xGroups[n])
		{
			UINT nLevel = m_xGroups[n]->GetLevel();
			packet.push(&nLevel, 4);
			
			packet1.push(m_xGroups[n]->GetName());
			packet1.push(1);
		}
	}
	packet.push((LPVOID)packet1.getbuf(), packet1.getsize());
	if (pPlayer == nullptr)
	{
		const void* pData = packet.getbuf();
		int nSize = packet.getsize();
		UINT nMemberCount = m_xMemberNameList.GetCount();
		for (UINT n = 0; n < nMemberCount; n++)
		{
			CGuildMemberEx* pMember = (CGuildMemberEx*)m_xMemberNameList[n]->lpObject;
			CHumanPlayer* p = nullptr;
			//封号编辑数据，只发给前10分组的成员
			if (pMember && pMember->GetGroup()->GetLevel() <= 10 && (p = pMember->GetRefPlayer()))
				p->SendMsg(0, 0xa11, 0, 0, 0, (LPVOID)pData, nSize);
		}
	}
	else
		pPlayer->SendMsg(0, 0xa11, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	
}

VOID CGuildEx::SendGuildTowerInfo(CHumanPlayer* pPlayer)
{
	xPacket packet(g_szTempBuffer, 65535);
	packet.push(2); //通灵塔坐标x
	packet.push(2); //通灵塔坐标y
	packet.push(1); //资源充足，通灵塔打开
	packet.push(14); //本体附加属性
	packet.push(14); //元神附加属性
	packet.push(1); //属性打开，个人贡献
	packet.push(GetFirstOwnerName()); //会长名字
	packet.push("/");
	char szText[2048]{};
	int offset = 0;
	for (int i = 1; i <= 10; i++)
	{
		// 在szText + offset位置开始写入
		offset += sprintf(szText + offset, "%d/%d/%d/%d/%d/%s/", i, nGuildOfficialCount[m_nLevel][i],
			nGuildOfficialCountMax[i], btGuildOfficialCrony[i], nGuildOfficialPrice[i], sGuildOfficial[i]);
	}
	packet.push(szText, offset);
	CGuildMemberEx* pGuildMember = GetMember(pPlayer);
	int pLevel = pGuildMember->GetGroup()->GetLevel();//玩家所在分组
	WORD wPermission = nGuildOfficialPermission[pLevel];
	if (!wPermission) wPermission = 1024;
	pPlayer->SendMsg(m_nGold, 0x340, m_nLevel, wPermission, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

CGuildMemberEx* CGuildEx::GetMember(const char* pszName)
{
	return (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pszName);
}

CGuildMemberEx* CGuildEx::GetMember(CHumanPlayer* pMember)
{
	return (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pMember->GetName());
}

BOOL CGuildEx::AddMember(const char* pszName)
{
	// 离线添加成员，没有玩家对象引用
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
	if (pObj != nullptr && AddMemberInternal(pszName, 0, 0))
	{
		char szTemp[64] = { 0 };
		o_strncpy(szTemp, pszName, 19);
		o_strncpy(szTemp + 20, GetName(), 31);
		pObj->SendMsg(0, DM_UPDATECHARGUILDNAME, 0, 0, 0, (LPVOID)szTemp, 52);
		return TRUE;
	}
	return FALSE;
}

BOOL CGuildEx::AddMember(CHumanPlayer* pMember)
{
	if (pMember == nullptr) return FALSE;
	if (pMember->GetGuild() != nullptr) return FALSE;
	BOOL bResult = AddMemberInternal(pMember->GetName(), pMember->GetPropValue(PI_LEVEL), pMember->GetPro(), pMember);
	if (bResult)
	{
		CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pMember->GetName());
		if (pGuildMember)
		{
			CGuildGroupEx* pGroup = pGuildMember->GetGroup();
			if (pGroup) pMember->SetGuild(this, pGroup->GetName(), pGroup->GetLevel());
		}
	}
	return bResult;
}

BOOL CGuildEx::AddMemberInternal(const char* pszName, WORD wLevel, BYTE btJob, CHumanPlayer* pRefPlayer)
{
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pszName);
	if (pGuildMember != nullptr) return FALSE;

	if (m_xMemberNameList.GetCount() >= m_nMaxMemberCount)
	{
		xError::setError(1001, "人数达到上限");
		return FALSE;
	}
	pGuildMember = m_xMemberPool.newObject();
	if (pGuildMember == nullptr) return FALSE;
	BOOL bNewGroup = FALSE;
	CGuildGroupEx* pGroup = nullptr;
	if (m_nGroupCount == 0)
	{
		pGroup = NewGroup(sGuildOfficial[1], 1);
		bNewGroup = TRUE;
	}
	else
		pGroup = GetGroupByLevel(99);

	if (pGroup == nullptr)
	{
		m_xMemberPool.deleteObject(pGuildMember);
		return FALSE;
	}

	pGuildMember->Init(pszName, 0, 0, wLevel, btJob, pRefPlayer);

	if (!pGroup->AddMember(pGuildMember, TRUE))
	{
		m_xMemberPool.deleteObject(pGuildMember);
		if (pGroup->GetCount() == 0)
			DeleteGroup(pGroup);
		return FALSE;
	}

	if (bNewGroup)
		AddGroupToList(pGroup);

	m_xMemberNameList.Add(pszName, (LPVOID)pGuildMember);
	Save();
	return TRUE;
}

BOOL CGuildEx::RemoveMember(const char* pszName)
{
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pszName);
	if (pGuildMember == nullptr)return FALSE;
	CGuildGroupEx* pGroup = pGuildMember->GetGroup();
	if (pGroup)
	{
		if (pGroup->GetLevel() == 1 && pGroup->GetCount() == 1)
		{
			xError::setError(2090, "不能通过删除会长来解散行会");
			return FALSE;
		}
		pGroup->RemoveMember(pGuildMember);
		if (pGroup->GetCount() == 0)
			DeleteGroupFromList(pGroup);
	}
	m_xMemberNameList.Delete(pszName);
	m_xMemberPool.deleteObject(pGuildMember);
	if (pGuildMember->IsRefValid())
		pGuildMember->GetRefPlayer()->SetGuild(nullptr);
	Save();
	return TRUE;
}

VOID CGuildEx::MemberLogon(CHumanPlayer* pMember)
{
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pMember->GetName());
	if (pGuildMember)
	{
		pGuildMember->SetRefPlayer(pMember);
		pMember->SetGuild(this, pGuildMember->GetGroup()->GetName(), pGuildMember->GetGroup()->GetLevel());
		for (UINT n = 0; n < m_nKillGuildCount; n++)
		{
			pMember->SaySystemAttrib(CC_GREEN, "%s 行会正在和您的行会进行行会战争", m_sKillGuilds[n].szName);
		}
	}
	else
		pMember->SetGuild(nullptr);
}

VOID CGuildEx::MemberLogoff(CHumanPlayer* pMember)
{
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pMember->GetName());
	if (pGuildMember)
	{
		pGuildMember->SetLevel(pMember->GetPropValue(PI_LEVEL)); // 玩家离线后，刷新等级
		pGuildMember->SetPro(pMember->GetPro()); // 玩家离线后，刷新职业
		pGuildMember->SetRefPlayer(nullptr);
		SendDurationMemberList();//玩家离线给所有人刷新
	}
}

BOOL CGuildEx::IsProperName(const char* pszName)
{
	char* p = (char*)pszName;
	if (*p >= '0' && *p <= '9') return FALSE;
	while (*p)
	{
		if (*p == '*' || *p == '\'' || *p == '\"' || ((BYTE)*p) <= 0x20 || *p == '\\' || *p == '/'
			|| *p == ':' || *p == '<' || *p == '>' || *p == '|' || *p == '?')
			return FALSE;
		p++;
	}
	return TRUE;
}

BOOL CGuildEx::Create(CHumanPlayer* pCreator, const char* pszName)
{
	if (pCreator->GetGuild() != nullptr)
	{
		pCreator->SaySystem("您已经加入一个行会了, 无法创建新的行会!");
		return FALSE;
	}
	if (!IsProperName(pszName))
	{
		pCreator->SaySystem("名字是非法的!");
		return FALSE;
	}
	o_strncpy(m_szName, pszName, 63);
	_makepath(m_szFilename, nullptr, ".\\Data\\GuildBase\\Guilds", m_szName, "ini");
	m_nMaxMemberCount = CGameWorld::GetInstance()->GetVar(EVI_STARTGUILDMEMBERCOUNT);
	if (!AddMember(pCreator))
	{
		pCreator->SaySystem("在把你添加进新的行会时遇到问题, 请稍候再试");
		return FALSE;
	}
	Init();//初始化行会
	return TRUE;
}

VOID CGuildEx::Init()
{
	m_nLevel = 1; //设置行会等级为1级
	AddGroupToList(NewGroup(sGuildOfficial[0], 99));//默认入会分组
	for (int i = 2; i <= 10; i++) //创建剩下默认行会分组
	{
		AddGroupToList(NewGroup(sGuildOfficial[i], i));
	}
	Save();
}

VOID CGuildEx::Save()
{
	char szFilename[1024];
	int len = (int)strlen(m_szFilename);
	o_strncpy(szFilename, m_szFilename, len);
	strcpy(szFilename + len - 3, "ini");

	FILE* fp = fopen(szFilename, "w");
	if (fp == nullptr)return;
	fprintf(fp, "[Guild]\nExp = %u\nLevel = %u\nMaxCount=%u\nRecruitState=%u\n", m_nExp, m_nLevel, m_nMaxMemberCount, m_boRecruitState);
	fclose(fp);

	strcpy(szFilename + len - 3, "txt");
	fp = fopen(szFilename, "w");
	if (fp == nullptr)return;

	fprintf(fp, "%s\n", CGameWorld::GetInstance()->GetName(ENI_GUILDNOTICE));
	int iptr = 0;
	BOOL bNewLine = TRUE;
	while (*(m_szNotice + iptr))
	{
		if (*(m_szNotice + iptr) == '\r')
		{
			fputc('\n', fp);
			bNewLine = TRUE;
		}
		else
		{
			if (bNewLine)
			{
				bNewLine = FALSE;
				fputc('+', fp);
			}
			fputc(*(m_szNotice + iptr), fp);
		}
		iptr++;
		if (iptr >= MAX_GUILD_NOTICE_LENGTH)
			break;
	}
	fputc('\n', fp);

	fprintf(fp, "%s\n", CGameWorld::GetInstance()->GetName(ENI_KILLGUILDS));
	fputc('\n', fp);

	fprintf(fp, "%s\n", CGameWorld::GetInstance()->GetName(ENI_ALLYGUILDS));
	for (UINT n = 0; n < m_nAllyGuildCount; n++)
	{
		fprintf(fp, "+%s\n", m_sAllyGuilds[n].szName);
	}
	fputc('\n', fp);

	fprintf(fp, "%s\n", CGameWorld::GetInstance()->GetName(ENI_MEMBERS));
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (m_xGroups[n])
		{
			fprintf(fp, "#%u %s\n", m_xGroups[n]->GetLevel(), m_xGroups[n]->GetName());
			m_xGroups[n]->SaveToFile(fp);
		}
	}
	fputc('\n', fp);

	fprintf(fp, "%s\n", CGameWorld::GetInstance()->GetName(ENI_GUILDDECLARATION));
	for (const auto& decl : m_DeclarationList)
	{
		fprintf(fp, "+%s\n", decl.c_str());
	}
	fputc('\n', fp);

	fprintf(fp, "%s\n", CGameWorld::GetInstance()->GetName(ENI_GUILDAPPLYLIST));
	for (UINT n = 0; n < m_xApplyPlayers.GetCount(); n++)
	{
		if (m_xApplyPlayers[n])
		{
			fprintf(fp, "+%s\n", m_xApplyPlayers[n]->pszString);
		}
	}
	fputc('\n', fp);

	fclose(fp);
	strcpy(szFilename + len - 3, "var");
	m_xVarList.SaveToFile(szFilename);
}

static CGuildGroupEx* g_xTempGroupList[100];
static CGuildMemberEx* g_xTempMemberList[1000];
BOOL CGuildEx::Load(const char* pszFile)
{
	CSettingFile sfGuild;
	if (!sfGuild.Open(pszFile))return FALSE;
	_splitpath(pszFile, nullptr, nullptr, m_szName, nullptr);
	if (m_szName[0] == 0)return FALSE;
	m_nLevel = (UINT)sfGuild.GetInteger("Guild", "Level", 1); // 行会等级默认值改成1
	m_nExp = (UINT)sfGuild.GetInteger("Guild", "Exp", 0);
	m_nMaxMemberCount = (UINT)sfGuild.GetInteger("Guild", "MaxCount", CGameWorld::GetInstance()->GetVar(EVI_STARTGUILDMEMBERCOUNT));
	m_boRecruitState = (BOOL)sfGuild.GetInteger("Guild", "RecruitState", 1);
	int len = (int)strlen(pszFile);
	char szGuildContentFile[260];
	if (_strnicmp(pszFile + len - 3, "ini", 3) != 0)
		return FALSE;
	o_strncpy(m_szFilename, pszFile, 1020);
	o_strncpy(szGuildContentFile, pszFile, 259);
	strcpy(szGuildContentFile + len - 3, "txt");
	CStringFile sfGuildContent(szGuildContentFile);
	if (sfGuildContent.GetLineCount() == 0)return FALSE;

	e_readguild rgstate = RG_NONE;

	static char* pszNotice = (char*)CGameWorld::GetInstance()->GetName(ENI_GUILDNOTICE);
	static char* pszKillGuilds = (char*)CGameWorld::GetInstance()->GetName(ENI_KILLGUILDS);
	static char* pszAllyGuilds = (char*)CGameWorld::GetInstance()->GetName(ENI_ALLYGUILDS);
	static char* pszDeclaration = (char*)CGameWorld::GetInstance()->GetName(ENI_GUILDDECLARATION);
	static char* pszMembers = (char*)CGameWorld::GetInstance()->GetName(ENI_MEMBERS);
	static char* pszApplyList = (char*)CGameWorld::GetInstance()->GetName(ENI_GUILDAPPLYLIST);

	m_szNotice[0] = 0;
	int iNoticePtr = 0;
	int nParam = 0;
	char* pLine = nullptr;
	UINT nCurLevel = 0;
	m_nAllyGuildCount = 0;
	memset(g_xTempGroupList, 0, sizeof(g_xTempGroupList));
	for (int i = 0; i < sfGuildContent.GetLineCount(); i++)
	{
		pLine = TrimEx(sfGuildContent[i]);
		if (*pLine == 0)continue;
		if (*pLine == '+')
		{
			pLine++;
			if (rgstate == RG_NOTICE)
			{
				len = (int)strlen(pLine);
				if (len + iNoticePtr + 1 >= MAX_GUILD_NOTICE_LENGTH)
					len = MAX_GUILD_NOTICE_LENGTH - iNoticePtr - 2;
				if (len <= 0)continue;
				o_strncpy(m_szNotice + iNoticePtr, pLine, len);
				iNoticePtr += len;
				m_szNotice[iNoticePtr++] = '\r';
				m_szNotice[iNoticePtr] = 0;
			}
			else if (rgstate == RG_KILLGUILDS)
			{
				o_strncpy(m_sKillGuilds[m_nKillGuildCount].szName, pLine, 63);
				m_sKillGuilds[m_nKillGuildCount].pGuild = nullptr;
				m_nKillGuildCount++;
			}
			else if (rgstate == RG_ALLYGUILDS)
			{
				o_strncpy(m_sAllyGuilds[m_nAllyGuildCount].szName, pLine, 63);
				m_sAllyGuilds[m_nAllyGuildCount].pGuild = nullptr;
				m_nAllyGuildCount++;
			}
			else if (rgstate == RG_MEMBERS)
			{
				if (nCurLevel == 0 || g_xTempGroupList[nCurLevel] == nullptr)
					continue;
				xStringsExtracter<5> s(pLine, "|", " \t");
				if (s.getCount() <= 0)continue;
				WORD wLevel = 0;
				wLevel = (DWORD)StringToInteger(s[1]);
				BYTE btJob = 0;
				btJob = (DWORD)StringToInteger(s[2]);
				DWORD dwExp = 0;
				if (s.getCount() > 3)
					dwExp = (DWORD)StringToInteger(s[3]);
				DWORD dwPower = 0;
				if (s.getCount() > 4)
					dwPower = (DWORD)StringToInteger(s[4]);
				CGuildMemberEx* pMember = m_xMemberPool.newObject();
				if (pMember == nullptr)continue;
				pMember->Init(s[0], dwExp, dwPower, wLevel, btJob, nullptr);
				if (!g_xTempGroupList[nCurLevel]->AddMember(pMember, TRUE))
					m_xMemberPool.deleteObject(pMember);
				m_xMemberNameList.Add(pMember->GetName(), (LPVOID)pMember);
			}
			else if (rgstate == RG_DECLARATION)
			{
				std::string decl(pLine);
				if (!decl.empty())
					m_DeclarationList.push_back(decl);
			}
			else if (rgstate == RG_APPLYLIST)
			{
				m_xApplyPlayers.Add(pLine);
			}
			else
				continue;
		}
		else if (*pLine == '#')
		{
			pLine++;
			if (rgstate != RG_MEMBERS)continue;

			xStringsExtracter<2> s(pLine, " \t", " \t", FALSE);
			if (s.getCount() < 2)continue;

			UINT nLevel = (UINT)StringToInteger(s[0]);
			if (nLevel == 0 || nLevel > 99)
				continue;
			g_xTempGroupList[nLevel] = NewGroup(s[1], nLevel);
			nCurLevel = nLevel;
		}
		else
		{
			if (_stricmp(pLine, pszNotice) == 0)
				rgstate = RG_NOTICE;
			else if (_stricmp(pLine, pszKillGuilds) == 0)
				rgstate = RG_KILLGUILDS;
			else if (_stricmp(pLine, pszAllyGuilds) == 0)
				rgstate = RG_ALLYGUILDS;
			else if (_stricmp(pLine, pszMembers) == 0)
				rgstate = RG_MEMBERS;
			else if (_stricmp(pLine, pszDeclaration) == 0)
				rgstate = RG_DECLARATION;
			else if (_stricmp(pLine, pszApplyList) == 0)
				rgstate = RG_APPLYLIST;
		}
	}
	m_nGroupCount = 0;
	for (UINT i = 0; i < 100; i++)
	{
		if (g_xTempGroupList[i])
		{
			m_xGroups[m_nGroupCount++] = g_xTempGroupList[i];
			g_xTempGroupList[i] = nullptr;
		}
	}
	strcpy(szGuildContentFile + len - 3, "var");
	m_xVarList.LoadFromFile(szGuildContentFile);
	PRINT(SUCCESS_GREEN, "行会 %s 已读取!\n", m_szName);
	return TRUE;
}

BOOL CGuildEx::AddApplyPlayer(CHumanPlayer* pOperator)
{
	char szbuffer[64];
	CSystemTime now;
	WORD wMonth = now.GetMonth();  // 获取月 (1-12)
	WORD wDay = now.GetDay();      // 获取日 (1-31)
	const char* szName = pOperator->GetName();
	int nLevel = pOperator->GetPropValue(PI_LEVEL);
	BYTE btJob = pOperator->GetPro();
	BYTE btSex = pOperator->GetSex();
	const char* pszSex = nullptr;
	switch (btSex)
	{
	case 0: pszSex = "男"; break;
	case 1: pszSex = "女"; break;
	default: pszSex = "男"; break;
	}
	sprintf(szbuffer, "%s#%d#%s#%u#%u-%u", szName, nLevel, pszSex, btJob, wMonth, wDay);
	if (m_xApplyPlayers.IndexOf(szbuffer) == -1)
		m_xApplyPlayers.Add(szbuffer);
	else
		return FALSE;
	Save();
	return TRUE;
}

BOOL CGuildEx::DelApplyPlayer(const char* pszCharName)
{
	UINT nCount = m_xApplyPlayers.GetCount();
	for (UINT n = 0; n < nCount; n++)
	{
		if (m_xApplyPlayers[n])
		{
			char* Params[1];
			int nParam = SearchParam(m_xApplyPlayers[n]->pszString, Params, 1, '#');
			if (nParam > 0 && strcmp(Params[0], pszCharName) == 0)
			{
				m_xApplyPlayers.Delete(n);
				Save();
				return TRUE;
			}
		}
	}
	return FALSE;
}

VOID CGuildEx::SendApplyList(CHumanPlayer* pOperator)
{
	xPacket packet(g_szTempBuffer, 65535);
	const char* s1C = "guildmgr";
	packet.push(s1C);
	packet.push(1);
	int nValue = 3;
	packet.push((LPVOID)&nValue, 1);
	UINT nCount = m_xApplyPlayers.GetCount();
	packet.push(&nCount, 4);
	for (UINT n = 0; n < nCount; n++)
	{
		if (m_xApplyPlayers[n])
		{
			packet.push(m_xApplyPlayers[n]->pszString);
			packet.push(1);
		}
	}
	pOperator->SendMsg(pOperator->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CGuildEx::SendWords(const char* pszWords)
{
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (m_xGroups[n])
			m_xGroups[n]->SendWords(pszWords);
	}
}

VOID CGuildEx::RefreshMemberName()
{
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (m_xGroups[n])
			m_xGroups[n]->UpdateNames();
	}
}

VOID CGuildEx::RefreshMemberTitle()
{
}

VOID CGuildEx::SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData, int iDataSize)
{
	int length = EncodeMsg(g_szTempBuffer, dwFlag, wCmd, w1, w2, w3, lpData, iDataSize);
	if (length > 0)
	{
		for (UINT n = 0; n < m_nGroupCount; n++)
		{
			if (m_xGroups[n])
				m_xGroups[n]->OnMsg(g_szTempBuffer, length);
		}
	}
}

UINT CGuildEx::GetMemberCountFor46Level()
{
	UINT nNum46 = 0;
	for (UINT n = 0; n < m_xMemberNameList.GetCount(); n++)
	{
		CGuildMemberEx* pMember = (CGuildMemberEx*)m_xMemberNameList[n]->lpObject;
		if (pMember && pMember->GetRefPlayer()->GetPropValue(PI_LEVEL) >= 46)
			nNum46++;
	}
	return nNum46;
}

BOOL CGuildEx::SendFirstPage(CHumanPlayer* pPlayer)
{
	xPacket packet(g_szTempBuffer, 65535);
	//	"测试行会\r \r0\r<Notice>\r大家好, 这里是测试行会\r没有任何功能, 只是给你看看, 哈哈\r<KillGuilds>假冒伪劣\r<AllyGuilds>KITT\r传世引擎\r"
	packet.push(GetName());
	if (IsMaster(pPlayer))
		packet.push("\r \r1\r<Notice>\r");
	else
		packet.push("\r \r0\r<Notice>\r");
	packet.push(m_szNotice);
	packet.push("\r<KillGuilds>");
	for (UINT n = 0; n < m_nKillGuildCount; n++)
	{
		if (m_sKillGuilds[n].szName[0])
		{
			packet.push(m_sKillGuilds[n].szName);
			packet.push("\r");
		}
	}
	packet.push("<AllyGuilds>");
	for (UINT n = 0; n < m_nAllyGuildCount; n++)
	{
		if (m_sAllyGuilds[n].szName[0])
		{
			packet.push(m_sAllyGuilds[n].szName);
			packet.push("\r");
		}
	}
	// 100是行会最大成员数
	pPlayer->SendMsg(0, 0x2f1, 100, 0, 1, (LPVOID)packet.getbuf(), packet.getsize());
	return TRUE;
}

BOOL CGuildEx::SendDurationMemberList(CHumanPlayer* pPlayer)
{
	//	"#1/*行会会长/会长1/#2/*行会成员/成员1/成员2/"
	char szBuffer[4096]{};
	xPacket packet1(szBuffer, 4096);
	xPacket packet(g_szTempBuffer, 65535);
	char szText[256];
	int length = 0;
	UINT nGroupCount = 0;//有成员的组数量
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (m_xGroups[n] && m_xGroups[n]->GetCount() > 0)
		{
			sprintf(szText, "#%u/*%s/", m_xGroups[n]->GetLevel(), m_xGroups[n]->GetName());
			packet1.push(szText);
			m_xGroups[n]->AppendDurationMembers(packet1);
			nGroupCount++;
		}
	}
	length = EncodeMsg((char*)packet.getfreebuf(), 0, 0x345, 0, 0, 0, (LPVOID)packet1.getbuf(), packet1.getsize());
	packet.addsize(length);
	//发送行会成员数据
	packet1.clear();
	packet1.push(&nGroupCount, 2);
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (m_xGroups[n] && m_xGroups[n]->GetCount() > 0)
		{
		    UINT uLevel = m_xGroups[n]->GetLevel();
			const char* szGroupName = m_xGroups[n]->GetName();
			WORD uCount = m_xGroups[n]->GetCount();
			packet1.push(&uLevel, 2);//分组序号
			packet1.push(szGroupName);//分组名
			packet1.push(1);
			packet1.push(&uCount, 2);//分组下人数
			m_xGroups[n]->AppendDurationMembers2(packet1);
		}
	}
	length = EncodeMsg((char*)packet.getfreebuf(), 0, 0x34a, 1, 0, 0, (LPVOID)packet1.getbuf(), packet1.getsize());
	packet.addsize(length);
	if (pPlayer == nullptr)
	{
		const char* pData = packet.getbuf();
		int nSize = packet.getsize();
		UINT nMemberCount = m_xMemberNameList.GetCount();
		for (UINT n = 0; n < nMemberCount; n++)
		{
			CGuildMemberEx* pMember = (CGuildMemberEx*)m_xMemberNameList[n]->lpObject;
			CHumanPlayer* p = nullptr;
			if (pMember && (p = pMember->GetRefPlayer()))
			{
				p->OnAroundMsg(nullptr, pData, nSize);
				p->UpdateViewName();
			}
		}
	}
	else
	{
		pPlayer->OnAroundMsg(nullptr, packet.getbuf(), packet.getsize());
		pPlayer->UpdateViewName();
	}
	return TRUE;
}

BOOL CGuildEx::SendExp(CHumanPlayer* pPlayer)
{
	//"测试行会/0/23784456/12525926/12525926"
	//"测试行会/等级/拥有行会经验值/贡献行会经验值/拥有行会能力值"
	char szText[256];
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pPlayer->GetName());
	if (pGuildMember == nullptr)return FALSE;
	sprintf(szText, "%s/%u/%u/%u/%u", GetName(), m_nLevel, m_nExp, pGuildMember->GetExp(), pGuildMember->GetPower());
	pPlayer->SendMsg(0, 0x2cd, m_xMemberNameList.GetCount(), m_nMaxMemberCount, 0, (LPVOID)szText);
	return TRUE;
}

BOOL CGuildEx::IsMember(CHumanPlayer* pPlayer)
{
	return (pPlayer->GetGuild() == this);
}

BOOL CGuildEx::IsMaster(CHumanPlayer* pPlayer)
{
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pPlayer->GetName());
	if (pGuildMember)
	{
		if (pGuildMember->GetGroup())
			return (pGuildMember->GetGroup()->GetLevel() == 1);
	}
	return FALSE;
}

BOOL CGuildEx::IsNoSay(const char* pszCharName)
{
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pszCharName);
	if (pGuildMember)
	{
		if (pGuildMember->GetGroup())
			return pGuildMember->IsNoSay();
	}
	return FALSE;
}

VOID CGuildEx::SetNoSay(const char* pszCharName)
{
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pszCharName);
	if (pGuildMember)
	{
		if (pGuildMember->GetGroup()) pGuildMember->SetNoSay();
	}
}

BOOL CGuildEx::IsAllyGuild(CGuildEx* pGuild) const
{
	if (pGuild == nullptr)return FALSE;
	for (UINT n = 0; n < m_nAllyGuildCount; n++)
	{
		if (m_sAllyGuilds[n].pGuild == pGuild)
			return TRUE;
	}
	return FALSE;
}

BOOL CGuildEx::IsAllyGuildMember(CHumanPlayer* pPlayer) const
{
	return IsAllyGuild(pPlayer->GetGuild());
}

BOOL CGuildEx::IsKillGuild(CGuildEx* pGuild) const
{
	if (pGuild == nullptr)return FALSE;
	for (UINT n = 0; n < m_nKillGuildCount; n++)
	{
		if (m_sKillGuilds[n].pGuild == pGuild)
			return TRUE;
	}
	return FALSE;
}

BOOL CGuildEx::IsKillGuildMember(CHumanPlayer* pPlayer) const
{
	return IsKillGuild(pPlayer->GetGuild());
}

BOOL CGuildEx::AddKillGuild(CGuildEx* pGuild)
{
	if (IsKillGuild(pGuild))
	{
		xError::setError(9220, "已经是敌对行会!");
		return FALSE;
	}
	if (m_nKillGuildCount >= 10)
	{
		xError::setError(2222, "达到敌对行会上限");
		return FALSE;
	}
	m_sKillGuilds[m_nKillGuildCount].pGuild = pGuild;
	o_strncpy(m_sKillGuilds[m_nKillGuildCount].szName, pGuild->GetName(), 60);
	m_nKillGuildCount++;
	return TRUE;
}

BOOL CGuildEx::RemoveKillGuild(CGuildEx* pGuild)
{
	for (UINT n = 0; n < m_nKillGuildCount; n++)
	{
		if (m_sKillGuilds[n].pGuild == pGuild)
		{
			m_nKillGuildCount--;
			for (UINT m = n; m < m_nKillGuildCount; m++)
			{
				m_sKillGuilds[m] = m_sKillGuilds[m + 1];
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CGuildEx::IsFirstMaster(CHumanPlayer* pPlayer)
{
	CGuildMemberEx* pGuildMember = (CGuildMemberEx*)m_xMemberNameList.ObjectOf(pPlayer->GetName());
	if (pGuildMember)
	{
		if (pGuildMember->GetGroup())
			return (pGuildMember->GetGroup()->GetLevel() == 1 &&
				pGuildMember->GetGroup()->GetFirstMember() == pGuildMember);
	}
	return FALSE;
}

CHumanPlayer* CGuildEx::GetMaster()
{
	CGuildGroupEx* pGrp = GetGroupByLevel(1);
	if (pGrp == nullptr) return nullptr;
	CGuildMemberEx* pMember = pGrp->GetFirstMember();
	if (pMember == nullptr) return nullptr;
	if (pMember->IsRefValid())
		return pMember->GetRefPlayer();
	return nullptr;
}

BOOL CGuildEx::BuildAlly(CGuildEx* pGuild)
{
	if (IsAllyGuild(pGuild))
	{
		xError::setError(2221, "已经是联盟");
		return FALSE;
	}
	if (m_nAllyGuildCount >= 10)
	{
		xError::setError(2222, "达到联盟行会上限");
		return FALSE;
	}
	m_sAllyGuilds[m_nAllyGuildCount].pGuild = pGuild;
	o_strncpy(m_sAllyGuilds[m_nAllyGuildCount].szName, pGuild->GetName(), 60);
	m_nAllyGuildCount++;
	return TRUE;
}

BOOL CGuildEx::BreakAlly(const char* pszName)
{
	for (UINT n = 0; n < m_nAllyGuildCount; n++)
	{
		if (strcmp(m_sAllyGuilds[n].szName, pszName) == 0)
		{
			m_nAllyGuildCount--;
			for (UINT m = n; m < m_nAllyGuildCount; m++)
			{
				m_sAllyGuilds[m] = m_sAllyGuilds[m + 1];
			}
			return TRUE;
		}
	}
	return FALSE;
}

//	0xfc 0x45
VOID CGuildEx::ReviewAroundNameColor()
{
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (m_xGroups[n])
			m_xGroups[n]->ReviewAroundNameColor();
	}
}

VOID CGuildEx::Clear()
{
	for (UINT n = 0; n < m_nGroupCount; n++)
	{
		if (m_xGroups[n])
		{
			m_xGroups[n]->ClearAllRef();
			m_xGroupPool.deleteObject(m_xGroups[n]);
		}
	}
	for (UINT n = 0; n < m_xMemberNameList.GetCount(); n++)
	{
		CGuildMemberEx* pMember = (CGuildMemberEx*)m_xMemberNameList[n]->lpObject;
		if (pMember)
		{
			pMember->setNode(nullptr);
			pMember->SetRefPlayer(nullptr);
			pMember->SetGroup(nullptr);
			m_xMemberPool.deleteObject(pMember);
			m_xMemberNameList[n]->lpObject = nullptr;
		}
	}
	m_xVarList.ClearVars();
	m_xMemberNameList.Clear();
	for (UINT n = 0; n < m_xApplyPlayers.GetCount(); n++)
	{
		m_xApplyPlayers[n]->lpObject = nullptr;
	}
	m_xApplyPlayers.Clear();
	m_DeclarationList.clear();
	memset(m_xGroups, 0, sizeof(m_xGroups));
	m_nGroupCount = 0;
	m_szName[0] = 0;
	m_szNotice[0] = 0;
	m_szFilename[0] = 0;
	m_nLevel = 0;
	m_nExp = 0;
	memset(m_sAllyGuilds, 0, sizeof(m_sAllyGuilds));
	m_nAllyGuildCount = 0;
	memset(m_sKillGuilds, 0, sizeof(m_sKillGuilds));
	m_nKillGuildCount = 0;
	m_fAttackSabuk = FALSE;
	m_nMaxMemberCount = 0;
	m_boAllNoSay = FALSE;
	m_boRecruitState = FALSE;
}

char* CGuildEx::GetVariableValue(const char* pszVariable)
{
	return m_xVarList.GetVarValue(pszVariable);
}

VOID CGuildEx::SetVariableValue(const char* pszVariable, const char* pszValue)
{
	m_xVarList.AddVar(pszVariable, (char*)pszValue);
}

VOID CGuildEx::ClrVariable(const char* pszVariable)
{
	m_xVarList.DelVar(pszVariable);
}

BOOL CGuildEx::AddVariable(const char* pszVariable, const char* pszValue)
{
	if (m_xVarList.GetVarValue(pszVariable) != nullptr)return FALSE;
	m_xVarList.AddVar(pszVariable, (char*)pszValue);
	return TRUE;
}

const char* CGuildEx::GetFirstOwnerName()
{
	CGuildGroupEx* pGrp = GetGroupByLevel(1);
	if (pGrp == nullptr)return "";
	CGuildMemberEx* pMember = pGrp->GetFirstMember();
	if (pMember == nullptr)return "";
	return pMember->GetName();
}