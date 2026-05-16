#pragma once
class CHumanPlayer;

class CGroupObject
{
public:
	CGroupObject(void);
	virtual ~CGroupObject(void);
	BOOL Create(CHumanPlayer* pLeader, CHumanPlayer* pFirstMember);
	BOOL AddMember(CHumanPlayer* pMember);
	BOOL IsMember(CHumanPlayer* pPlayer);
	BOOL IsMemberDBId(DWORD dwId);
	BOOL IsLeader(CHumanPlayer* pPlayer);
	VOID DelMember(CHumanPlayer* pMember);
	VOID LeaveMember(CHumanPlayer* pMember);
	VOID SendMsg(CHumanPlayer* pSender, DWORD dwFlag, WORD wCmd, WORD wParam1, WORD wParam2, WORD wParam3, LPVOID lpData = nullptr, int datasize = -1);
	VOID UpdateMemberPosition(CHumanPlayer* p);
	VOID DestroyGroup();
	VOID AdjustGroupExp(CHumanPlayer* p, DWORD dwExp, DWORD dwId);
	VOID SaySystemAttrib(DWORD dwAttrib, const char* pszMsg, ...);
	int	GetCount() { return m_xMembersArray.GetCount(); }
	xAutoPtrArray<CHumanPlayer>& GetMemberArray() { return m_xMembersArray; }
	xAutoPtrArray<CHumanPlayer>* GetArrayPtr() { return &m_xMembersArray; }
protected:
	VOID UpdateNameList();
	xAutoPtrArray<CHumanPlayer> m_xMembersArray;
};