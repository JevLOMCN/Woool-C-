#pragma once
enum e_sendmsg_rage
{
	SMR_AREA,
	SMR_AROUND,
	SMR_MAP,
	SMR_SERVER,
	SMR_WORLD,
	SMR_GROUP,
	SMR_GUILD,
};

enum e_gm_command;
class CHumanPlayer;

class CClientObj :
	public CClientObject
{
public:
	CClientObj(void);
	virtual ~CClientObj(void);
	VOID Clean();
	VOID Update();
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	VOID OnDBMsg(PMIRMSG pMsg, int datasize);
	VOID OnVerifyString(const char* pszString);
	VOID OnDisconnect();
	VOID OnConnection();
	VOID OnDBItem(DBITEM* pItemArray, int nCount, BYTE btFlag);
public:
	VOID SendActionResult(int x, int y, BOOL bSuccess);
	VOID SendAddItem(ITEM& item);
	VOID SendEquipItemResult(BOOL bSuccess, int pos, DWORD dwMakeIndex);
	VOID SendUnEquipItemResult(BOOL bSuccess, int pos, DWORD dwMakeIndex);
	int	GetGmLevel()const { return m_iGmLevel; }
	const char* GetAccount() { return m_EnterInfo.szAccount; }
	VOID OnCreateItem(ITEM& item, int pos, BYTE btFlag);
	int GetPacketQueueCount() { return m_xPacketQueue.getcount(); }
private:
	VOID SendClientNewMail(WORD Parm1, WORD Parm2, WORD Parm3);
	VOID SendBagItems(DBITEM* pItems, int count);
	VOID SendEquipments( /*DBITEM * pItems, int count*/);
	VOID ProcClientMsg(PMIRMSG pMsg, int datasize);
	VOID OnCommand(const char* pszCommand);
private:
	ENTERGAMESERVER	m_EnterInfo;
	gameserverusermode m_State;
	CHumanPlayer* m_pPlayer;
	int	m_iGmLevel;
	BOOL m_bScrollTextMode;
	BOOL m_bNoticeMode;
	BOOL m_bCompetlyQuit;
};