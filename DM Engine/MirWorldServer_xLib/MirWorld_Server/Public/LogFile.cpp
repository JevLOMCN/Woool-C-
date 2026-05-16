#include "StdAfx.h"
#include "inc.h"
#include ".\logfile.h"

CLogFile::CLogFile(void)
{
	m_fpLog = nullptr;
	memset(&m_stFileTime, 0, sizeof(m_stFileTime));
	memset(m_szCache, 0, sizeof(m_szCache));
	memset(m_szFile, 0, sizeof(m_szFile));

	if (!PathIsFolder("..\\日志"))
	{
		if (!CreateDirectory("..\\日志", nullptr))
		{
			MessageBox(GetDesktopWindow(), "创建日志文件夹失败, 请关闭程序手动创建!", "提示", 0);
		}
	}
}

CLogFile::~CLogFile(void)
{
	if (m_fpLog != nullptr)
		fclose(m_fpLog);
}

BOOL CLogFile::Init(const char* pszPath)
{
	if (!PathIsFolder(pszPath))
	{
		if (!CreateDirectory(pszPath, nullptr))
			return FALSE;
	}
	o_strncpy(m_szFile, pszPath, 1023);
	ChangeLogFile();
	return TRUE;
}

VOID CLogFile::LogDate()
{
	SYSTEMTIME	st;
	GetLocalTime(&st);
	LogText("[%04u-%02u-%02u %02u:%02u:%02u] ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

VOID CLogFile::LogText(const char* pszString, ...)
{
	THREAD_PROTECT;
	if (m_fpLog == nullptr)return;
	va_list	vl;
	va_start(vl, pszString);
	vsprintf(m_szCache, pszString, vl);
	va_end(vl);
	fputs(m_szCache, m_fpLog);
	fflush(m_fpLog);
}

VOID CLogFile::LogTextRt(const char* pszString, ...)
{
	THREAD_PROTECT;
	if (m_fpLog == nullptr)return;
	va_list	vl;
	va_start(vl, pszString);
	vsprintf(m_szCache, pszString, vl);
	va_end(vl);
	fputs(m_szCache, m_fpLog);
	fputs("\n", m_fpLog);
	fflush(m_fpLog);
}

VOID CLogFile::LogTextWithDate(const char* pszString, ...)
{
	THREAD_PROTECT;
	if (m_fpLog == nullptr)return;
	LogDate();
	va_list	vl;
	va_start(vl, pszString);
	vsprintf(m_szCache, pszString, vl);
	va_end(vl);
	fputs(m_szCache, m_fpLog);
	fflush(m_fpLog);
}

VOID CLogFile::LogBinary(LPVOID lpData, int nSize, int nLineSize)
{
	THREAD_PROTECT;
	BYTE* pString = (BYTE*)lpData;
	char szAscii[20] = { 0 };
	szAscii[1] = 0;
	if (m_fpLog == nullptr || lpData == nullptr)return;

	LogDate();
	NextLine();

	fprintf(m_fpLog, "binary data( 0x%p ) size: %d\n<start>\n", lpData, nSize);
	int iLineCount = 0;
	for (int i = 0;i < nSize;i++)
	{
		if (*(pString + i) <= 0x20)
		{
			fputc('.', m_fpLog);
		}
		else
		{
			fputc(*(pString + i), m_fpLog);
		}
		iLineCount++;
		if (iLineCount >= nLineSize)
		{
			fprintf(m_fpLog, "\n");
			iLineCount = 0;
		}
	}
	fprintf(m_fpLog, "\n<end>\n");
	fflush(m_fpLog);

	fprintf(m_fpLog, "hex code of binary data\n<start>\n");
	for (int i = 0;i < nSize;i++)
	{
		fprintf(m_fpLog, "%02x ", *(pString + i));
		iLineCount++;
		if (iLineCount >= nLineSize)
		{
			fprintf(m_fpLog, "\n");
			iLineCount = 0;
		}
	}
	fprintf(m_fpLog, "\n<end>\n");
	fflush(m_fpLog);
}

VOID CLogFile::NextLine()
{
	if (m_fpLog != 0)
		fprintf(m_fpLog, "\n");
}

VOID CLogFile::ChangeLogFile()
{
	THREAD_PROTECT;
	SYSTEMTIME stNow;
	CriticalSectionProtector m_Protector;
	GetLocalTime(&stNow);
	if (stNow.wDay != m_stFileTime.wDay || m_fpLog == nullptr)
	{
		CHAR szFile1[256];
		CHAR szFile[1024];
		sprintf(szFile1, "%04u-%02u-%02u.log", stNow.wYear, stNow.wMonth, stNow.wDay);
		_makepath(szFile, nullptr, m_szFile, szFile1, nullptr);
		if (m_fpLog)fclose(m_fpLog);
		m_fpLog = fopen(szFile, "a");
		m_stFileTime = stNow;
	}
}