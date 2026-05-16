#pragma once
#define	LGERR CLogFile::GetInstance()->LogTextWithDate( "´íÎó @ %s (%d) => ", __FILE__, __LINE__ );\
			CLogFile::GetInstance()->LogTextRt
#define	LG1 CLogFile::GetInstance()->LogText
#define	LG2 CLogFile::GetInstance()->LogTextWithDate

class CLogFile : public xSingletonClass<CLogFile>
{
public:
	CLogFile(void);
	virtual ~CLogFile(void);
	BOOL Init(const char* pszPath);
	VOID NextLine();
	VOID LogDate();
	VOID LogText(const char* pszString, ...);
	VOID LogTextRt(const char* pszString, ...);
	VOID LogTextWithDate(const char* pszString, ...);
	VOID LogBinary(LPVOID lpData, int nSize, int nLineSize = 80);
	VOID ChangeLogFile();
private:
	FILE* m_fpLog;
	SYSTEMTIME	m_stFileTime;
	char m_szFile[1024];
	char m_szCache[16384];
	THREAD_PROTECT_DEFINE;
};