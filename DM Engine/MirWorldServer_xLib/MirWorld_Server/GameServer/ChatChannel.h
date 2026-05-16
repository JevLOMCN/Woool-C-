#pragma once

class CHumanPlayer;

class CChatChannel
{
public:
	enum e_operation
	{
		CCO_SENDMSG,
	};
	typedef struct tag_operation
	{
		e_operation opr;
		int	iParam;
		LPVOID	lpParam;
	}OPERATION;
	CChatChannel(void);
	virtual ~CChatChannel(void);

	void	Clean();

	BOOL	Add( CHumanPlayer * pPlayer );
	VOID	Remove( CHumanPlayer * pPlayer );

	void	OnRemove( CHumanPlayer * pPlayer );
	void OnAdd( CHumanPlayer * pPlayer );
	void	OnWalkThrough( CHumanPlayer * pPlayer );
	
	int	GetLimitCount(){return m_nLimitCount;}
	void	SetLimitCount( int nLimitCount ){ m_nLimitCount = nLimitCount;}

	VOID	SendChannelMsg( DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData = nullptr, int datasize = -1 );
protected:
	static VOID	CALLBACK	fnWalkThrough( CHumanPlayer * pPlayer, CChatChannel * pChannel );
	xAutoList	m_xList;
	int	m_nLimitCount;
	OPERATION	m_WTOperation;
};
