#pragma once
class CInputListener
{
public:
	virtual void OnInput(const char* pszString) = 0;
};

enum e_ColorType
{
	TEXT_WHITE, // Èá°×É«
	COOL_BLUE, // ¾²Ú×À¶
	STRING_GREEN, // »îÁ¦ÂÌ
	WARN_YELLOW, // Ã÷ÑÞ»Æ
	KEYWORD_PINK, // Éºº÷³È
	FUNC_PURPLE, // µ­ÑÅ×Ï
	CYAN, //Ó«¹âÇà
	ORANGE, //µÍ±¥ºÍ³È
	SUCCESS_GREEN, // ÏÊÂÌÉ«
	ERROR_RED, // ´¿ºìÉ«
};

class CIOConsole
{
public:
	CIOConsole(void);
	virtual ~CIOConsole(void);
	// TEXT_WHITE Èá°×É« 
	// COOL_BLUE ¾²Ú×À¶ 
	// STRING_GREEN »îÁ¦ÂÌ 
	// WARN_YELLOW Ã÷ÑÞ»Æ 
	// KEYWORD_PINK Éºº÷³È 
	// FUNC_PURPLE µ­ÑÅ×Ï 
	// CYAN Ó«¹âÇà 
	// ORANGE µÍ±¥ºÍ³È
	// SUCCESS_GREEN ÏÊÂÌÉ«
	// ERROR_RED ´¿ºìÉ«
	void OutPut(DWORD dwColor, const char* pszString, ...);
	DWORD GetColor(DWORD index);
public:
	virtual void OutPutStatic(DWORD dwColor, const char* pszString) {};
	virtual void Input(const char* pszString) { if (m_pInputListener != nullptr)m_pInputListener->OnInput(pszString); }
	CInputListener* GetInputListener() { return m_pInputListener; }
	void	SetInputListener(CInputListener* pInputListener) { m_pInputListener = pInputListener; }
protected:
	CInputListener* m_pInputListener;
};
