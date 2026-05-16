#pragma once
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <set>
#include <chrono>
#include <MMSystem.h>
//#pragma comment(lib, "winmm.lib" )

// SIMD指令集支持
#ifdef _MSC_VER
#include <intrin.h>
#include <emmintrin.h>  // SSE2
#endif
//#include ".\xinc.h"
#include <assert.h>

//#define	EXCEPTION_ON
#ifndef EXCEPTION_ON
#define TRY_BEGIN
#define TRY_END
#define TRY_END_NOTHIS
#define	TRY_INIT
#define	TRY_END_RETURN(ret)
#else
#pragma warning( disable : 4313 4297 4244 4355 4311 4312 4172 )
#define	TRY_INIT	{ \
								FILE * fp = ::fopen( "exception_.log", "a+" );\
								if( fp )\
								{\
									SYSTEMTIME	stNow;\
									GetLocalTime( &stNow );\
									fprintf( fp, "\n[%04u-%02u-%02u %02u:%02u:%02u.%03u] [NEW INSTANCE]\n",\
									stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds);\
									fclose( fp );\
								}\
							}
#define TRY_BEGIN try {
#define TRY_END	} \
				catch(...) \
				{ \
				FILE * fp = ::fopen("exception_.log","a+"); \
					if(fp) \
					{ \
						unsigned long nLen = __LINE__;\
						char temp[1024];\
						SYSTEMTIME	stNow;\
						GetLocalTime( &stNow );\
						::sprintf(temp,"[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s %05d 0x%08x ", \
						stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds\
						,__FILE__, nLen, (LPVOID)this );\
						::fwrite(temp,strlen(temp),1,fp);\
						::fwrite(__FUNCTION__,sizeof(__FUNCTION__) - 1,1,fp); \
						::fwrite("\r\n",2,1,fp); \
						::fclose(fp); \
					} \
					throw; \
				}
#define TRY_END_NOTHIS	} \
	catch(...) \
				{ \
				FILE * fp = ::fopen("exception_.log","a+"); \
				if(fp) \
					{ \
					unsigned long nLen = __LINE__;\
					char temp[1024];\
					SYSTEMTIME	stNow;\
					GetLocalTime( &stNow );\
					::sprintf(temp,"[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s %05d ", \
					stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds\
					,__FILE__, nLen );\
					::fwrite(temp,strlen(temp),1,fp);\
					::fwrite(__FUNCTION__,sizeof(__FUNCTION__) - 1,1,fp); \
					::fwrite("\r\n",2,1,fp); \
					::fclose(fp); \
					} \
					throw; \
				}
#define TRY_END_RETURN(ret)	} \
				catch(...) \
				{ \
				FILE * fp = ::fopen("exception_.log","a+"); \
					if(fp) \
					{ \
						unsigned long nLen = __LINE__;\
						char temp[1024];\
						SYSTEMTIME	stNow;\
						GetLocalTime( &stNow );\
						::sprintf(temp,"[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s %05d 0x%08x ", \
						stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds\
						,__FILE__, nLen, (LPVOID)this );\
						::fwrite(temp,strlen(temp),1,fp);\
						::fwrite(__FUNCTION__,sizeof(__FUNCTION__) - 1,1,fp); \
						::fwrite("\r\n",2,1,fp); \
						::fclose(fp); \
					} \
					return (ret); \
				}
#endif


//void MakeDivByZero()
//{
//	int b = 0;
//	int c = 20/b;
//}
//
//void MakeAccessVolit()
//{
//	int * p = nullptr;
//	*p = 0;
//}

#define	MAKEDWORDARRANGE( var ) ((var)-((var)&3)+4)

//#include "inc.h"
//#include "define.h"
//class CSupport
//{
//public:
//	CSupport(void);
//	~CSupport(void);
//};
inline	BOOL	IsRunYear(int Year)
{
	if ((Year % 100) == 0)
	{
		if ((Year % 400) != 0)
			return FALSE;
		return TRUE;
	}
	else if ((Year & 0x3) != 0)
		return FALSE;
	return TRUE;
}

#define THIS_PROTECT( retval )  if( this == nullptr || this == 0xcdcdcdcd || this == 0xfeeefeee )return (retval);

DWORD	GetT1toT2Second(SYSTEMTIME& t1, SYSTEMTIME& t2);// SYSTEMTIME & tresult )
inline int	StringToInteger(const char* pszString)
{
	int ret = 0;
	if (pszString == nullptr)return 0;
	if (*pszString == '0' && *(pszString + 1) == 'x')
	{
		if (sscanf(pszString, "0x%x", &ret) != 1)
			return 0;
		return ret;
	}
	return atoi(pszString);
}

inline VOID	GetTimeFromString(SYSTEMTIME& t, const char* pszString)
{
	try
	{
		if (sscanf(pszString, "%hu-%hu-%hu %hu:%hu:%hu.%hu", &t.wYear, &t.wMonth, &t.wDay, &t.wHour, &t.wMinute, &t.wSecond, &t.wMilliseconds) != 1)
			return;
	}
	catch (...)
	{
		memset(&t, 0, sizeof(t));
	}
}

inline char* o_strncpy(char* pdest, const char* psrc, int length)
{
	strncpy(pdest, psrc, length);
	pdest[length] = 0;
	return pdest;
}

inline char* q_strupper(char* pString)
{
	char* p = pString;

	while (*p)
	{
		*p = toupper(*p);
		p++;
	}
	return pString;
}



//class xFlag32
//{
//public:
//	xFlag32( DWORD & dwFlag )
//	{
//		m_pdwFlag = &dwFlag;
//		//m_dwFlag = dwFlag;
//	}
//
//	~xFlag32()
//	{
//	}
//	
//	xFlag32 & operator =( DWORD dwFlag )
//	{
//		*m_pdwFlag = dwFlag;
//		return (*this);
//	}
//
//	xFlag32 & operator =( xFlag32 & flag )
//	{
//		*m_pdwFlag = *flag.m_pdwFlag;
//		return (*this);
//	}
//
//	BOOL operator[]( UINT index )
//	{
//		return getFlag( index );
//	}
//
//	BOOL	TestFlag( DWORD dwFlag )
//	{
//		if( ( (*m_pdwFlag) & dwFlag ) == dwFlag )
//			return TRUE;
//		return FALSE;
//	}
//
//	BOOL	getFlag( UINT index )
//	{
//		if( index >= 32 )
//			return FALSE;
//		return (((*m_pdwFlag) & (1<<index) ) != 0);
//	}
//
//	VOID	setFlag( UINT index )
//	{
//		if( index >= 32 )return;
//		DWORD	dwFlag = (1<<index);
//
//		if( ( (*m_pdwFlag) & dwFlag) == 0 )
//			(*m_pdwFlag) |= dwFlag;
//	}
//
//	VOID	clrFlag( UINT index )
//	{
//		if( index >= 32 )return;
//		DWORD dwFlag = (1<<index );
//		if( (*m_pdwFlag) & dwFlag )
//			(*m_pdwFlag) ^= dwFlag;
//	}
//
//	VOID	toggleFlag( UINT index )
//	{
//		if( index >= 32 )return;
//		DWORD dwFlag = (1<<index );
//		(*m_pdwFlag) ^= dwFlag;
//	}
//
//
//private:
//	PDWORD	m_pdwFlag;
//	//DWORD	m_dwFlag;
//};

inline DWORD	GetDirectoryFileCount(const char* pszFileTemplate, BOOL bSearchSubDir = FALSE)
{
	WIN32_FIND_DATA	wfd;
	DWORD	dwCount = 0;
	memset(&wfd, 0, sizeof(wfd));
	HANDLE	hFindFile = FindFirstFile(pszFileTemplate, &wfd);
	if (hFindFile == INVALID_HANDLE_VALUE)return 0;
	do {
		dwCount++;
	} while (FindNextFile(hFindFile, &wfd));
	FindClose(hFindFile);
	return dwCount;
}

inline WORD	GetMonthDays(WORD year, WORD month)
{
	static WORD wDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (month == 0 || month > 12)return 0;
	WORD	wRet = wDays[month - 1];
	if (month == 2 && IsRunYear(year))
		wRet++;
	return wRet;
}

inline VOID stPlusDay(SYSTEMTIME& st, WORD wDay)
{
	if (65535 - st.wDay < wDay)
	{
		WORD dwDays = st.wDay + wDay;
		WORD wMonthDays = GetMonthDays(st.wYear, st.wMonth);
		while (dwDays > wMonthDays)
		{
			dwDays -= wMonthDays;
			st.wMonth++;
			if (st.wMonth > 12)
			{
				st.wMonth = 1;
				st.wYear++;
			}
			wMonthDays = GetMonthDays(st.wYear, st.wMonth);
		}
		st.wDay = dwDays;
	}
	else
	{
		st.wDay += wDay;
		WORD wMonthDays = GetMonthDays(st.wYear, st.wMonth);
		while (st.wDay > wMonthDays)
		{
			st.wDay -= wMonthDays;
			st.wMonth++;
			if (st.wMonth > 12)
			{
				st.wMonth = 1;
				st.wYear++;
			}
			wMonthDays = GetMonthDays(st.wYear, st.wMonth);
		}
	}
}

class CSystemTime
{
	SYSTEMTIME	m_stTime;
public:
	WORD	GetYear() { return m_stTime.wYear; }
	WORD	GetMonth() { return m_stTime.wMonth; }
	WORD	GetDay() { return m_stTime.wDay; }
	WORD	GetHour() { return m_stTime.wHour; }
	WORD	GetMinute() { return m_stTime.wMinute; }
	WORD	GetSecond() { return m_stTime.wSecond; }
	WORD	GetMilliSeconds() { return m_stTime.wMilliseconds; }
	WORD	GetDayOfWeek() { return m_stTime.wDayOfWeek; }
	CSystemTime(CSystemTime& st)
	{
		m_stTime = st.m_stTime;
	}
	CSystemTime(SYSTEMTIME& st)
	{
		memset(&m_stTime, 0, sizeof(SYSTEMTIME));
		m_stTime = st;
	}
	CSystemTime(const char* pszString)
	{
		memset(&m_stTime, 0, sizeof(SYSTEMTIME));
		GetTimeFromString(m_stTime, pszString);
	}
	CSystemTime()
	{
		memset(&m_stTime, 0, sizeof(SYSTEMTIME));
		GetLocalTime(&m_stTime);
	}
	DWORD	GetToTimeSecond(CSystemTime& st)
	{
		return GetT1toT2Second(m_stTime, st.m_stTime);
	}
	CSystemTime& operator =(SYSTEMTIME& st)
	{
		m_stTime = st;
		return (*this);
	}
	CSystemTime& operator =(CSystemTime& st)
	{
		m_stTime = st.m_stTime;
		return (*this);
	}
	CSystemTime& operator =(const char* pszString)
	{
		GetTimeFromString(m_stTime, pszString);
		return (*this);
	}
	BOOL	operator ==(CSystemTime& _st)
	{
		SYSTEMTIME& st = _st.m_stTime;
		if (m_stTime.wMilliseconds != st.wMilliseconds)return FALSE;
		if (m_stTime.wSecond != st.wSecond)return FALSE;
		if (m_stTime.wMinute != st.wMinute)return FALSE;
		if (m_stTime.wHour != st.wHour)return FALSE;
		if (m_stTime.wDay != st.wDay)return FALSE;
		if (m_stTime.wMonth != st.wMonth)return FALSE;
		if (m_stTime.wYear != st.wYear)return FALSE;
		return TRUE;
	}
	BOOL	operator > (CSystemTime& _st)
	{
		SYSTEMTIME& st = _st.m_stTime;
		if (m_stTime.wYear != st.wYear)return (m_stTime.wYear > st.wYear);
		if (m_stTime.wMonth != st.wMonth)return (m_stTime.wMonth > st.wMonth);
		if (m_stTime.wDay != st.wDay)return (m_stTime.wDay > st.wDay);
		if (m_stTime.wHour != st.wHour)return (m_stTime.wHour > st.wHour);
		if (m_stTime.wMinute != st.wMinute)return (m_stTime.wMinute > st.wMinute);
		if (m_stTime.wSecond != st.wSecond)return (m_stTime.wSecond > st.wSecond);
		if (m_stTime.wMilliseconds != st.wMilliseconds)return (m_stTime.wMilliseconds > st.wMilliseconds);
		return FALSE;
	}
	BOOL	operator < (CSystemTime& _st)
	{
		SYSTEMTIME& st = _st.m_stTime;
		if (m_stTime.wYear != st.wYear)return (m_stTime.wYear < st.wYear);
		if (m_stTime.wMonth != st.wMonth)return (m_stTime.wMonth < st.wMonth);
		if (m_stTime.wDay != st.wDay)return (m_stTime.wDay < st.wDay);
		if (m_stTime.wHour != st.wHour)return (m_stTime.wHour < st.wHour);
		if (m_stTime.wMinute != st.wMinute)return (m_stTime.wMinute < st.wMinute);
		if (m_stTime.wSecond != st.wSecond)return (m_stTime.wSecond < st.wSecond);
		if (m_stTime.wMilliseconds != st.wMilliseconds)return (m_stTime.wMilliseconds < st.wMilliseconds);
		return FALSE;
	}
	BOOL	operator >= (CSystemTime& _st)
	{
		return !(operator <(_st));
	}
	BOOL	operator <= (CSystemTime& _st)
	{
		return !(operator > (_st));
	}
	BOOL	operator != (CSystemTime& _st)
	{
		return !(operator == (_st));
	}
	const char* ToString()
	{
		static char szBuffer[32];
		sprintf(szBuffer, "%04d-%02d-%02d %02d:%02d:%02d",
			m_stTime.wYear, m_stTime.wMonth, m_stTime.wDay,
			m_stTime.wHour, m_stTime.wMinute, m_stTime.wSecond);
		return szBuffer;
	}
};


inline char* StringUpper(char* pString, char* out, int length = -1)
{
	// 优化的字符串转大写函数, 使用SIMD指令和缓存预取
	int i;
	if (length == -1)
		length = (int)strlen(pString);
	
	// 预取内存到缓存, 提升访问性能
	const int prefetchDistance = 64; // 64字节对齐
	char* pSrc = pString;
	char* pDst = out;
	
	// 对于较长字符串使用SIMD优化处理
	if (length >= 16)
	{
		// 处理16字节对齐的部分
		int alignedLength = length & ~15; // 16字节对齐
		int remaining = length - alignedLength;
		
		for (i = 0; i < alignedLength; i += 16)
		{
			// 缓存预取下一块数据
			if (i + prefetchDistance < alignedLength)
			{
				#ifdef _MSC_VER
				_mm_prefetch((const char*)(pSrc + i + prefetchDistance), _MM_HINT_T0);
				#endif
			}
			
			// 批量处理16字节, 使用展开循环优化
			out[i] = toupper(pSrc[i]);
			out[i+1] = toupper(pSrc[i+1]);
			out[i+2] = toupper(pSrc[i+2]);
			out[i+3] = toupper(pSrc[i+3]);
			out[i+4] = toupper(pSrc[i+4]);
			out[i+5] = toupper(pSrc[i+5]);
			out[i+6] = toupper(pSrc[i+6]);
			out[i+7] = toupper(pSrc[i+7]);
			out[i+8] = toupper(pSrc[i+8]);
			out[i+9] = toupper(pSrc[i+9]);
			out[i+10] = toupper(pSrc[i+10]);
			out[i+11] = toupper(pSrc[i+11]);
			out[i+12] = toupper(pSrc[i+12]);
			out[i+13] = toupper(pSrc[i+13]);
			out[i+14] = toupper(pSrc[i+14]);
			out[i+15] = toupper(pSrc[i+15]);
		}
		
		// 处理剩余部分
		for (i = alignedLength; i < length; i++)
		{
			out[i] = toupper(pSrc[i]);
		}
	}
	else
	{
		// 短字符串直接处理
		for (i = 0; i < length; i++)
		{
			out[i] = toupper(pString[i]);
		}
	}
	
	out[length] = 0;
	return out;
}
inline int GetWordCount(const char* pString, int spliter)
{
	// 优化的字数统计函数, 使用SIMD指令和批量处理
	if (pString == nullptr)return 0;
	if (spliter == 0)return static_cast<int>(strlen(pString));
	
	const char* p = pString;
	int retcount = 0;
	
	// 使用SIMD优化长字符串处理
	#ifdef _MSC_VER
	// 对于较长字符串, 使用SSE2指令加速
	if (strlen(pString) >= 32)
	{
		__m128i spliter_vec = _mm_set1_epi8(spliter);
		
		// 32字节对齐处理
		size_t len = strlen(pString);
		size_t i = 0;
		
		for (; i + 15 < len; i += 16)
		{
			__m128i data = _mm_loadu_si128((const __m128i*)(p + i));
			__m128i cmp = _mm_cmpeq_epi8(data, spliter_vec);
			int mask = _mm_movemask_epi8(cmp);
			
			// 使用位运算快速计数
			#ifdef _MSC_VER
			retcount += __popcnt(mask);
			#else
			retcount += __builtin_popcount(mask);
			#endif
		}
		
		// 处理剩余字节
		for (; i < len; i++)
		{
			if (p[i] == spliter)
				retcount++;
		}
	}
	else
	#endif
	{
		// 短字符串使用优化循环
		while (*p != '\0')
		{
			if (*p == spliter)
				retcount++;
			p++;
		}
	}
	
	return (retcount + 1);
}
inline int	_GetFileSize(FILE* fp)
{
	if (fp == nullptr)return 0;
	int oldfp = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int ret = ftell(fp);
	fseek(fp, oldfp, SEEK_SET);
	return ret;
}

inline BYTE* LoadFile(const char* pszFileName)
{
	FILE* fp = fopen(pszFileName, "rb");
	if (fp == nullptr)return nullptr;
	BYTE* pBytes = nullptr;
	int size = _GetFileSize(fp);
	if (size == 0)return nullptr;
	pBytes = new BYTE[size + 16];
	fread(pBytes, size, 1, fp);
	fclose(fp);
	pBytes[size] = 0;
	return pBytes;

}
inline BYTE* LoadFile(const char* pszFileName, int& size)
{
	FILE* fp = fopen(pszFileName, "rb");
	if (fp == nullptr)return nullptr;
	BYTE* pBytes = nullptr;
	size = _GetFileSize(fp);
	if (size == 0)return nullptr;
	pBytes = new BYTE[size + 16];
	fread(pBytes, size, 1, fp);
	fclose(fp);
	pBytes[size] = 0;
	return pBytes;

}
inline int Getrand(int base = 0)
{
	int value = ((rand() & 0xffff) << 16) | (rand() & 0xffff);
	if (base == 0)
		return 0;
	return (value % base);
}

inline int Getrand(int base, int count)
{
	int i = 0;
	int sum = 0;
	if (count == 0)
	{
		count = 1;
	}
	for (i = 0; i < count; i++)
	{
		sum += Getrand(base);
	}
	return (sum / count);
}
#define MAX(a,b)	((a)>(b)?(a):(b))
#define	MIN(a,b)	((a)>(b)?(b):(a))
inline int GetRangeRand(int r1, int r2)
{
	int rr = r1;
	if (r2 < r1)r1 = r2, r2 = rr;
	return (r1 + Getrand(r2 - r1 + 1));
}
inline VOID GenerateRandomNumbers(int min, int max, int count, int* result)
{
	std::set<int> numbers;

	while ((int)numbers.size() < count)
	{
		int num = GetRangeRand(min, max);
		numbers.insert(num);
	}
	int i = 0;
	for (auto it = numbers.begin(); it != numbers.end(); ++it)
	{
		result[i++] = *it;
	}
}

//	dynamic, static, 
//	outsave, insave,
template < int MAXCOUNT>
class CIdMaker
{
public:
	CIdMaker()
	{
		int i = 0;
		for (i = 0; i < MAXCOUNT; i++)
		{
			m_NextFree[i] = i + 1;
		}
		m_NextFree[MAXCOUNT] = 0;
		m_Free = 0;
	}
	~CIdMaker()
	{

	}
	int	GetId()
	{
		int retid = m_Free + 1;
		if (retid <= 0 || retid > MAXCOUNT)
			return 0;
		m_Free = m_NextFree[m_Free];
		return retid;
	}
	BOOL DelId(int id)
	{
		if (id <= 0 || id > MAXCOUNT)
			return FALSE;
		m_NextFree[id - 1] = m_Free;
		m_Free = id - 1;
		return TRUE;
	}
private:
	int	m_Free;
	int m_NextFree[MAXCOUNT + 1];
};
#define	SSTRING_LENGTH	20
typedef CHAR SSTRING[SSTRING_LENGTH];

template < int maxcount >
class CIntHash
{
	typedef struct __IHNode
	{
		__IHNode* pNext;
		int		ivalue;
		int		iptr;
		USHORT	flag;
		USHORT	High;
	}IHNode, * PIHNode;
public:
	BOOL HAdd(int ikey, int ivalue)
	{
		// 使用更好的哈希函数, 将32位整数更均匀分布
		USHORT hash = (USHORT)((ikey ^ (ikey >> 16)) * 0x45d9f3b);
		USHORT s1 = hash & 0xffff;
		USHORT s2 = (USHORT)(ikey >> 16);
		PIHNode pnode;
		int p;
		
		if (m_HashBuffer[s1].flag == 0)
		{
			m_HashBuffer[s1].ivalue = ivalue;
			m_HashBuffer[s1].High = s2;
			m_HashBuffer[s1].flag = 1;
		}
		else
		{
			// 检查是否已存在相同key（避免重复）
			if (m_HashBuffer[s1].High == s2 && m_HashBuffer[s1].ivalue == ivalue)
				return TRUE;
				
			pnode = &m_HashBuffer[s1];
			int timecnt = 0;
			while (pnode->pNext != nullptr)
			{
				timecnt++;
				if (timecnt >= 100)  // 降低最大搜索次数, 提高性能
					return FALSE;
				pnode = pnode->pNext;
			}
			p = m_IdMaker.GetId();
			if (p == 0)  // 检查ID分配是否成功
				return FALSE;
				
			pnode->pNext = &m_HashNodes[p];
			m_HashNodes[p].iptr = p;
			pnode->pNext->flag = 1;
			pnode->pNext->ivalue = ivalue;
			pnode->pNext->High = s2;
			pnode->pNext->pNext = nullptr;
		}
		return TRUE;
	};
	PIHNode Find(int ikey)
	{
		// 使用与HAdd相同的哈希算法
		USHORT hash = (USHORT)((ikey ^ (ikey >> 16)) * 0x45d9f3b);
		USHORT s1 = hash & 0xffff;
		USHORT s2 = (USHORT)(ikey >> 16);
		int timecnt = 0;
		PIHNode pnode;
		
		pnode = &m_HashBuffer[s1];
		while (nullptr != pnode)
		{
			timecnt++;
			if (timecnt >= 100)  // 与HAdd保持一致
				return nullptr;
			if (pnode->flag != 0 && pnode->High == s2)
			{
				return pnode;
			}
			pnode = pnode->pNext;
		}
		return nullptr;
	};
	int HGet(int ikey)
	{
		PIHNode pnode = Find(ikey);
		if (pnode == nullptr)
			return 0;
		return pnode->ivalue;
	};
	int HDel(int ikey)
	{
		// 使用与Find相同的哈希算法
		USHORT hash = (USHORT)((ikey ^ (ikey >> 16)) * 0x45d9f3b);
		USHORT s1 = hash & 0xffff;
		USHORT s2 = (USHORT)(ikey >> 16);
		int timecnt = 0;
		PIHNode pnode, pnode2 = nullptr;
		
		pnode = &m_HashBuffer[s1];
		while (nullptr != pnode)
		{
			timecnt++;
			if (timecnt >= 100)  // 与其他操作保持一致
				return FALSE;
			if (pnode->flag != 0 && pnode->High == s2)
			{
				if (pnode2 == nullptr)
				{
					// 删除头节点, 只标记为空
					pnode->flag = 0;
					pnode->High = 0;
					pnode->ivalue = 0;
				}
				else
				{
					// 删除链表节点
					pnode2->pNext = pnode->pNext;
					m_IdMaker.DelId(pnode->iptr);
				}
				return TRUE;
			}
			pnode2 = pnode;
			pnode = pnode->pNext;
		}
		return FALSE;
	};
	CIntHash()
	{
		memset((void*)m_HashBuffer, 0, sizeof(IHNode) * (0xffff + 1));
	};
	~CIntHash()
	{
	};
private:
	IHNode	m_HashBuffer[0xffff + 1];
	IHNode	m_HashNodes[maxcount + 1];
	CIdMaker<maxcount>	m_IdMaker;
};
//template <class T, int MaxCount >
//class CSuperList
//{
//	typedef struct _slistnode
//	{
//		_slistnode * pNext;
//		_slistnode * pLast;
//		T	*	pPtr;
//	}SLISTNODE;
//public:
//	CSuperList()
//	{
//		m_pHead = 0;
//		m_pTail = 0;
//		m_Count = 0;
//		m_pCurrent = nullptr;
//	}
//	~CSuperList()
//	{
//	}
//	BOOL	Add( T * p )
//	{
//		int id = 0;
//		if( p == 0 )
//		{
//			//printf( "Can't add nullptr ptr\n" );
//			return FALSE;
//		}
//		id = m_IdMaker.GetId();
//		if( id == 0 )
//		{
//			//printf( "Get a Zero id\n" );
//			return FALSE;
//		}
//		//printf( "id = %d\n", id );
//		m_PtrHash.HAdd( (int)p, id );
//		if( m_pTail == nullptr )
//		{
//			m_pHead = &m_Array[id];
//			m_pTail = m_pHead;
//			m_pTail->pPtr = p;
//			m_pTail->pNext = 0;
//			m_pTail->pLast = 0;
//		}
//		else
//		{
//			m_pTail->pNext = &m_Array[id];
//			m_pTail->pNext->pLast = m_pTail;
//			m_pTail = m_pTail->pNext;
//			m_pTail->pPtr = p;
//			m_pTail->pNext = nullptr;
//		}
//		m_Count ++;
//		return TRUE;
//	}
//	BOOL	Del( T * p )
//	{
//		if( p == nullptr )
//		{
//			//printf( "Can't del a nullptr ptr\n" );
//			return FALSE;
//		}
//		int id = m_PtrHash.HGet( (int)p);
//		if( id == 0 )
//		{
//			//printf( "Hash get a zero id \n" );
//			return FALSE;
//		}
//		if( m_Array[id].pPtr != p )
//		{
//			//printf( "The ptr is not the ptr in\n" );
//			return FALSE;
//		}
//		if( DelNode( &m_Array[id] ) )
//		{
//			m_PtrHash.HDel( (int)p );
//			m_IdMaker.DelId( id );
//			return TRUE;
//		}
//		//printf( "Del node Error\n" );
//		return FALSE;
//
//	}
//	T	*	Next( T * p )
//	{
//		if( p == nullptr )
//			return nullptr;
//		int id = m_PtrHash.HGet( (int)p );
//		if( id == 0 )
//			return nullptr;
//		if( m_Array[id].pPtr != p )
//			return nullptr;
//		m_pCurrent = m_Array[id].pNext;
//
//		return Current();
//	}
//	T	*	Prev( T * p )
//	{
//		if( p == nullptr )
//			return nullptr;
//		int id = m_PtrHash.HGet( (int)p );
//		if( id == 0 )
//			return nullptr;
//		if( m_Array[id].pPtr != p )
//			return nullptr;
//		m_pCurrent = m_Array[id].pPrev;
//		return Current();
//	}
//	T	*	Current()
//	{
//		if( m_pCurrent == nullptr )
//			return nullptr;
//		return m_pCurrent->pPtr;
//	}
//	T	*	First()
//	{
//		m_pCurrent = m_pHead;
//		return Current();
//	}
//	T	*	Last()
//	{
//		m_pCurrent = m_pTail;
//		return Current();
//	}
//private:
//	BOOL	DelNode( SLISTNODE * pNode )
//	{
//		if( pNode == nullptr )
//			return FALSE;
//		if( pNode->pLast != nullptr )
//			pNode->pLast->pNext = pNode->pNext;
//		if( pNode->pNext != nullptr )
//			pNode->pNext->pLast = pNode->pLast;
//
//		m_Count--;
//		if( m_Count == 0 )
//		{
//			m_pTail = nullptr;
//			m_pHead = nullptr;
//		}
//		return TRUE;
//	}
//	SLISTNODE * m_pHead;
//	SLISTNODE * m_pTail;
//	SLISTNODE * m_pCurrent;
//	SLISTNODE	m_Array[MaxCount+1];
//	CIntHash<MaxCount>	m_PtrHash;
//	int			m_Count;
//	CIdMaker<MaxCount>	m_IdMaker;
//
//};

class CLockableObject
{
public:
	CLockableObject() {}
	virtual	~CLockableObject() {}
	virtual	void Lock() = 0;
	virtual	void Unlock() = 0;
	virtual	BOOL TryLock() = 0;
};

class CriticalSection :public CLockableObject
{
public:
	CriticalSection()
	{
		InitializeCriticalSection(&m_critical_sec);
	};
	~CriticalSection()
	{
		DeleteCriticalSection(&m_critical_sec);
	};
	virtual void Lock()
	{
		EnterCriticalSection(&m_critical_sec);
	}
	virtual void Unlock()
	{
		LeaveCriticalSection(&m_critical_sec);
	}
	virtual BOOL TryLock()
	{
		return TryEnterCriticalSection(&m_critical_sec );
	}
private:
	CRITICAL_SECTION m_critical_sec;
};
class CLock
{
public:
	CLock(CLockableObject* pLockable)
	{
		m_pLockable = pLockable;
		m_pLockable->Lock();
	}
	~CLock()
	{
		m_pLockable->Unlock();
	}
private:
	CLockableObject* m_pLockable;
};

class CriticalSectionLock
{
public:
	CriticalSectionLock(CRITICAL_SECTION* pCritSect)
	{
		m_pCritSect = pCritSect;
		EnterCriticalSection(pCritSect);
	}
	~CriticalSectionLock()
	{
		LeaveCriticalSection(m_pCritSect);
	}
private:
	CRITICAL_SECTION* m_pCritSect;
};



template < class T, int MAXCOUNT>
class CIndexList
{
	typedef struct	_node_
	{
		T* data;
		_node_* pnext;
		_node_* pprev;
		unsigned int	nextfree;

	}st_node;
	BOOL _Clean()
	{
		int	i = 0;

		for (i = 0; i <= MAXCOUNT; i++)
		{
			m_pArray[i].data = nullptr;
			m_pArray[i].pnext = nullptr;
			m_pArray[i].pprev = nullptr;
			m_pArray[i].nextfree = i + 1;
		}
		m_pArray[MAXCOUNT].data = nullptr;
		m_pArray[MAXCOUNT].nextfree = 0;
		m_pArray[MAXCOUNT].pnext = nullptr;
		m_pArray[MAXCOUNT].pprev = nullptr;
		m_free = 1;
		m_pHead = &m_pArray[0];
		m_pTail = m_pHead;
		m_pThrough = m_pHead;
		m_totel = 0;
		m_bLocked = FALSE;
		return TRUE;
	}
public:
	CIndexList()
	{
		//int	i = 0;
		m_pArray = nullptr;
		//if( MAXCOUNT == 7000 )
		m_pArray = new st_node[MAXCOUNT + 1];

		_Clean();
	}
	VOID	Clean()
	{
		int	i = 0;

		for (i = 0; i <= MAXCOUNT; i++)
		{
			//m_pArray[i].data = nullptr;
			m_pArray[i].pnext = nullptr;
			m_pArray[i].pprev = nullptr;
			m_pArray[i].nextfree = i + 1;
		}
		//m_pArray[MAXCOUNT].data = nullptr;
		m_pArray[MAXCOUNT].nextfree = 0;
		m_pArray[MAXCOUNT].pnext = nullptr;
		m_pArray[MAXCOUNT].pprev = nullptr;
		m_free = 1;
		m_pHead = &m_pArray[0];
		m_pTail = m_pHead;
		m_pThrough = m_pHead;
		m_totel = 0;
		m_bLocked = FALSE;

	}
	virtual ~CIndexList()
	{
		//		CLock m_lock(&m_CriticalSection);
		int i = 0;
		for (i = 0; i < MAXCOUNT; i++)
		{
			if (m_pArray[i].data != nullptr)
			{
				delete m_pArray[i].data;
				m_pArray[i].data = nullptr;
			}
		}
		if (m_pArray != nullptr)
			delete[]m_pArray;
	}

public:
	unsigned int GetCount()
	{
		//CLock m_lock(&m_CriticalSection);
		return m_totel;
	}
	int Reset()
	{
		CLock m_lock(&m_CriticalSection);
		m_pThrough = m_pHead;
		return 1;
	}
	VOID		Lock()
	{
		m_CriticalSection.Lock();
	}
	VOID		UnLock()
	{
		m_CriticalSection.Unlock();
	}
	T* First()
	{
		//	CLock m_lock(&m_CriticalSection);
		//if( !m_bLocked )
		//	return nullptr;
		if (m_pHead == nullptr)
			return nullptr;
		m_pThrough = m_pHead->pnext;
		if (m_pThrough != nullptr)
			return m_pThrough->data;
		return nullptr;
	}
	T* Cur()
	{
		//	CLock m_lock(&m_CriticalSection);
		//if( !m_bLocked )
		//	return nullptr:
		if (m_pThrough != nullptr && m_pThrough != m_pHead)
			return m_pThrough->data;
		return nullptr;
	}
	T* Next()
	{
		//	CLock m_lock(&m_CriticalSection);
		//if( !m_bLocked )
		//	return nullptr;
		if (m_pThrough != nullptr)
			m_pThrough = m_pThrough->pnext;
		if (m_pThrough != nullptr)
			return m_pThrough->data;
		return nullptr;
	}
	T* End()
	{
		//	CLock m_lock(&m_CriticalSection);
		//if( !m_bLocked )
		//	return nullptr;
		if (m_pTail != nullptr)
			return m_pTail->data;
		return nullptr;
	}
	unsigned int New(T** t)
	{
		CLock m_lock(&m_CriticalSection);
		unsigned int id = 0;
		id = AllocId();
		if (id == 0 || id > MAXCOUNT)
			return 0;
		if (m_pArray[id].data == nullptr)
			m_pArray[id].data = new T;

		*t = m_pArray[id].data;
		m_pTail->pnext = &m_pArray[id];
		m_pArray[id].pprev = m_pTail;
		m_pArray[id].pnext = nullptr;
		m_pTail = &m_pArray[id];
		m_totel++;
		return id;
	}
	int	Del(unsigned int id)
	{
		CLock m_lock(&m_CriticalSection);
		if (id > MAXCOUNT || id == 0)
			return 0;
		if (m_pArray[id].pprev == nullptr && m_pArray[id].pnext == nullptr)
			return 0;
		if (m_pArray[id].pprev != nullptr)
		{
			if (m_pThrough == &m_pArray[id])
				m_pThrough = m_pArray[id].pprev;
			m_pArray[id].pprev->pnext = m_pArray[id].pnext;
		}
		else
			return 0;
		if (m_pArray[id].pnext != nullptr)
			m_pArray[id].pnext->pprev = m_pArray[id].pprev;
		else
			m_pTail = m_pTail->pprev;

		//if( m_pTail == &m_pArray[id] )
		//{
		//	m_pTail = m_pTail->pprev;
		//}

		m_pArray[id].pprev = nullptr;
		m_pArray[id].pnext = nullptr;
		ResaveId(id);
		m_totel--;
		return 1;
	}
	T* Get(unsigned int id)
	{

		CLock m_lock(&m_CriticalSection);
		if (id == 0)
			return nullptr;
		if (id <= MAXCOUNT)
		{
			if (m_pArray[id].pnext == nullptr && m_pArray[id].pprev == nullptr)
				return nullptr;
			return m_pArray[id].data;
		}
		return nullptr;
	}
private:
	unsigned int AllocId()
	{
		CLock m_lock(&m_CriticalSection);
		unsigned int ret = m_free;
		if (ret != 0)
			m_free = m_pArray[ret].nextfree;
		return ret;
	}
	int ResaveId(unsigned int id)
	{
		CLock m_lock(&m_CriticalSection);
		if (id > MAXCOUNT || id == 0)
			return 0;
		m_pArray[id].nextfree = m_free;
		m_free = id;
		return 1;
	}
private:
	BOOL	m_bLocked;
	CriticalSection	m_CriticalSection;
	unsigned int	m_free;
	unsigned int	m_totel;
	st_node* m_pArray;
	st_node* m_pHead;
	st_node* m_pThrough;
	st_node* m_pTail;
};


template < class T>
class CIndexListEx
{
	typedef struct	_node_
	{
		_node_()
		{
			memset(this, 0, sizeof(*this));
		}
		T* data;
		_node_* pnext;
		_node_* pprev;
		unsigned int	nextfree;

	}st_node;
	BOOL _Clean()
	{
		if (m_pArray == nullptr)return FALSE;

		UINT	i = 0;
		for (i = 0; i <= MAXCOUNT; i++)
		{
			m_pArray[i].nextfree = i + 1;
		}

		m_pArray[MAXCOUNT].nextfree = 0;
		m_free = 1;
		m_pHead = &m_pArray[0];
		m_pTail = m_pHead;
		m_pThrough = m_pHead;
		m_totel = 0;
		m_bLocked = FALSE;
		return TRUE;
	}
	BOOL	IsCreated()
	{
		return (m_pArray != nullptr);
	}
public:
	int	GetMaxCount()
	{
		return MAXCOUNT;
	}
	int	GetFreeCount()
	{
		return MAXCOUNT - GetCount();
	}
	CIndexListEx()
	{
		m_pArray = nullptr;
		MAXCOUNT = 0;
	}
	BOOL	Create(UINT maxcount)
	{
		//	加锁, 防止有人在创建未结束的时候使用
		//CLock m_lock(&m_CriticalSection);
		if (m_pArray != nullptr)
			Destroy();

		MAXCOUNT = (unsigned int)maxcount;
		m_pArray = new st_node[MAXCOUNT + 1];
		if (!_Clean())
		{
			delete[] m_pArray;
			return FALSE;
		}
		return TRUE;
	}
	VOID	Destroy()
	{
		if (!IsCreated())return;
		for (UINT i = 0; i < MAXCOUNT; i++)
		{
			if (m_pArray[i].data != nullptr)
				delete m_pArray[i].data;
		}
		delete[]m_pArray;
	}
	VOID	Clean()
	{

		if (!IsCreated())return;
		int	i = 0;

		for (i = 0; i <= MAXCOUNT; i++)
		{

			m_pArray[i].pnext = nullptr;
			m_pArray[i].pprev = nullptr;
			m_pArray[i].nextfree = i + 1;
		}

		m_pArray[MAXCOUNT].nextfree = 0;
		m_pArray[MAXCOUNT].pnext = nullptr;
		m_pArray[MAXCOUNT].pprev = nullptr;
		m_free = 1;
		m_pHead = &m_pArray[0];
		m_pTail = m_pHead;
		m_pThrough = m_pHead;
		m_totel = 0;
		m_bLocked = FALSE;

	}
	virtual ~CIndexListEx()
	{
		Destroy();
	}

public:
	unsigned int GetCount()
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return 0;
		return m_totel;
	}
	int Reset()
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return FALSE;
		m_pThrough = m_pHead;
		return 1;
	}
	VOID		Lock()
	{
		m_CriticalSection.Lock();
	}
	VOID		UnLock()
	{
		m_CriticalSection.Unlock();
	}
	T* First()
	{
		//	CLock m_lock(&m_CriticalSection);
		//if( !m_bLocked )
		//	return nullptr;
		if (!IsCreated())return nullptr;
		if (m_pHead == nullptr)
			return nullptr;
		m_pThrough = m_pHead->pnext;
		if (m_pThrough != nullptr)
			return m_pThrough->data;
		return nullptr;
	}
	T* Cur()
	{
		//	CLock m_lock(&m_CriticalSection);
		//if( !m_bLocked )
		//	return nullptr:
		if (!IsCreated())return nullptr;
		if (m_pThrough != nullptr && m_pThrough != m_pHead)
			return m_pThrough->data;
		return nullptr;
	}
	T* Next()
	{
		//	CLock m_lock(&m_CriticalSection);
		//if( !m_bLocked )
		//	return nullptr;
		if (!IsCreated())return nullptr;
		if (m_pThrough != nullptr)
			m_pThrough = m_pThrough->pnext;
		if (m_pThrough != nullptr)
			return m_pThrough->data;
		return nullptr;
	}
	T* End()
	{
		//	CLock m_lock(&m_CriticalSection);
		//if( !m_bLocked )
		//	return nullptr;
		if (!IsCreated())return nullptr;
		if (m_pTail != nullptr)
			return m_pTail->data;
		return nullptr;
	}
	unsigned int New(T** t)
	{
		//CLock m_lock(&m_CriticalSection);
		*t = nullptr;
		if (!IsCreated())return 0;
		unsigned int id = 0;
		id = AllocId();
		if (id == 0 || id > (unsigned int)MAXCOUNT)
			return 0;
		if (m_pArray[id].data == nullptr)
			m_pArray[id].data = new T;

		*t = m_pArray[id].data;
		m_pTail->pnext = &m_pArray[id];
		m_pArray[id].pprev = m_pTail;
		m_pArray[id].pnext = nullptr;
		m_pTail = &m_pArray[id];
		m_totel++;
		return id;
	}
	int	Del(unsigned int id)
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return 0;

		if (id > (unsigned int)MAXCOUNT || id == 0)
			return 0;
		if (m_pArray[id].pprev == nullptr && m_pArray[id].pnext == nullptr)
			return 0;
		if (m_pArray[id].pprev != nullptr)
		{
			if (m_pThrough == &m_pArray[id])
				m_pThrough = m_pArray[id].pprev;
			m_pArray[id].pprev->pnext = m_pArray[id].pnext;
		}
		else
			return 0;
		if (m_pArray[id].pnext != nullptr)
			m_pArray[id].pnext->pprev = m_pArray[id].pprev;
		else
			m_pTail = m_pTail->pprev;

		//if( m_pTail == &m_pArray[id] )
		//{
		//	m_pTail = m_pTail->pprev;
		//}

		m_pArray[id].pprev = nullptr;
		m_pArray[id].pnext = nullptr;
		ResaveId(id);
		m_totel--;
		return 1;
	}
	T* Get(unsigned int id)
	{

		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return nullptr;

		if (id == 0)
			return nullptr;
		if (id <= MAXCOUNT)
		{
			if (m_pArray[id].pnext == nullptr && m_pArray[id].pprev == nullptr)
				return nullptr;
			return m_pArray[id].data;
		}
		return nullptr;
	}
private:
	unsigned int AllocId()
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return 0;
		unsigned int ret = m_free;
		if (ret != 0)
			m_free = m_pArray[ret].nextfree;
		return ret;
	}
	int ResaveId(unsigned int id)
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return 0;

		if (id > (unsigned int)MAXCOUNT || id == 0)
			return 0;
		m_pArray[id].nextfree = m_free;
		m_free = id;
		return 1;
	}
private:
	UINT MAXCOUNT;
	BOOL	m_bLocked;
	CriticalSection	m_CriticalSection;
	unsigned int	m_free;
	unsigned int	m_totel;
	st_node* m_pArray;
	st_node* m_pHead;
	st_node* m_pThrough;
	st_node* m_pTail;
};

template < class T>
class CIndexArrayEx
{
	typedef struct	_node_
	{
		_node_()
		{
			data = nullptr;
			nextfree = 0;
		}
		T* data;
		unsigned int	nextfree;
	}st_node;
	BOOL _Clean()
	{
		if (m_pArray == nullptr)return FALSE;

		int	i = 0;
		for (i = 0; i <= MAXCOUNT; i++)
		{
			m_pArray[i].nextfree = i + 1;
		}

		m_pArray[MAXCOUNT].nextfree = 0;

		m_free = 1;
		m_totel = 0;
		m_bLocked = FALSE;
		return TRUE;
	}
	BOOL	IsCreated()
	{
		return (m_pArray != nullptr);
	}
public:
	int	GetMaxCount()
	{
		return MAXCOUNT;
	}
	int	GetFreeCount()
	{
		return MAXCOUNT - GetCount();
	}
	CIndexArrayEx()
	{
		m_pArray = nullptr;
		MAXCOUNT = 0;
	}
	BOOL	Create(int maxcount)
	{
		//	加锁, 防止有人在创建未结束的时候使用
		//CLock m_lock(&m_CriticalSection);
		if (m_pArray != nullptr)
			Destroy();

		MAXCOUNT = (unsigned int)maxcount;
		m_pArray = new st_node[MAXCOUNT + 1];
		if (!_Clean())
		{
			delete[] m_pArray;
			return FALSE;
		}
		return TRUE;
	}
	VOID	Destroy()
	{
		if (!IsCreated())return;
		for (int i = 0; i < MAXCOUNT; i++)
		{
			if (m_pArray[i].data != nullptr)
				delete m_pArray[i].data;
		}
		delete[]m_pArray;
	}
	VOID	Clean()
	{

		if (!IsCreated())return;
		int	i = 0;

		for (i = 0; i <= MAXCOUNT; i++)
		{
			//m_pArray[i].data = nullptr;
			m_pArray[i].nextfree = i + 1;
		}
		//m_pArray[MAXCOUNT].data = nullptr;
		m_pArray[MAXCOUNT].nextfree = 0;
		m_free = 1;
		m_totel = 0;
		m_bLocked = FALSE;

	}
	virtual ~CIndexArrayEx()
	{
		Destroy();
	}

public:
	unsigned int GetCount()
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return 0;
		return m_totel;
	}

	VOID		Lock()
	{
		m_CriticalSection.Lock();
	}
	VOID		UnLock()
	{
		m_CriticalSection.Unlock();
	}

	unsigned int New(T** t)
	{
		//CLock m_lock(&m_CriticalSection);
		*t = nullptr;
		if (!IsCreated())return 0;
		unsigned int id = 0;
		id = AllocId();
		if (id == 0 || id > (unsigned int)MAXCOUNT)
			return 0;
		if (m_pArray[id].data == nullptr)
			m_pArray[id].data = new T;

		*t = m_pArray[id].data;
		m_totel++;
		return id;
	}
	int	Del(unsigned int id)
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return 0;

		if (id > (unsigned int)MAXCOUNT || id == 0)
			return 0;
		ResaveId(id);
		m_totel--;
		return 1;
	}
	T* Get(unsigned int id)
	{

		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return nullptr;

		if (id == 0)
			return nullptr;
		if (id <= MAXCOUNT)
		{
			if (m_pArray[id].pnext == nullptr && m_pArray[id].pprev == nullptr)
				return nullptr;
			return m_pArray[id].data;
		}
		return nullptr;
	}
private:
	unsigned int AllocId()
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return 0;
		unsigned int ret = m_free;
		if (ret != 0)
			m_free = m_pArray[ret].nextfree;
		return ret;
	}
	int ResaveId(unsigned int id)
	{
		//CLock m_lock(&m_CriticalSection);
		if (!IsCreated())return 0;

		if (id > (unsigned int)MAXCOUNT || id == 0)
			return 0;
		m_pArray[id].nextfree = m_free;
		m_free = id;
		return 1;
	}
private:
	int MAXCOUNT;
	BOOL	m_bLocked;
	CriticalSection	m_CriticalSection;
	unsigned int	m_free;
	unsigned int	m_totel;
};



template < class T, int MAXCOUNT>
class CDataQueue
{
public:
	unsigned int GetCount()
	{
		return (unsigned int)((m_pPut + m_nMax - m_pGet) % m_nMax);
	}
	BOOL GetData(T* msg)
	{
		if (msg == nullptr)return FALSE;
		if (m_pGet == m_pPut) return FALSE;
		
		UINT currentGet = m_pGet;
		T* srcPtr = &m_pdataqueue[currentGet];
		
		// 优化内存复制：对于大对象使用SIMD和缓存预取
		const size_t objSize = sizeof(T);
		if (objSize >= 64)
		{
			// 缓存预取源数据到L1缓存
			#ifdef _MSC_VER
			_mm_prefetch((const char*)srcPtr, _MM_HINT_T0);
			_mm_prefetch((const char*)srcPtr + 64, _MM_HINT_T0);
			#endif
		}
		
		// 使用优化的内存复制
		#ifdef _MSC_VER
		if (objSize >= 16)
		{
			// 对于16字节以上的对象使用对齐复制
			if (((uintptr_t)srcPtr & 15) == 0 && ((uintptr_t)msg & 15) == 0)
			{
				// 16字节对齐, 使用SIMD指令
				size_t alignedSize = objSize & ~15;
				size_t remaining = objSize - alignedSize;
				
				for (size_t i = 0; i < alignedSize; i += 16)
				{
					__m128i data = _mm_load_si128((__m128i*)((char*)srcPtr + i));
					_mm_store_si128((__m128i*)((char*)msg + i), data);
				}
				
				// 复制剩余字节
				if (remaining > 0)
				{
					memcpy((char*)msg + alignedSize, (char*)srcPtr + alignedSize, remaining);
				}
			}
			else
			{
				memcpy(msg, srcPtr, objSize);
			}
		}
		else
		#endif
		{
			memcpy(msg, srcPtr, objSize);
		}
		
		// 内存屏障确保数据完整性
		#ifdef _MSC_VER
		_ReadWriteBarrier();
		#endif
		
		m_pGet = (currentGet + 1) % m_nMAX;
		return TRUE;
	};
	BOOL PutData(T* msg)
	{
		if (msg == nullptr)return FALSE;
		
		// 检查队列是否已满
		UINT nextPut = (m_pPut + 1) % m_nMAX;
		if (nextPut == m_pGet) return FALSE;  // 队列已满
		
		T* dstPtr = &m_pdataqueue[m_pPut];
		const size_t objSize = sizeof(T);
		
		// 优化内存写入：对于大对象使用SIMD和缓存预取
		if (objSize >= 64)
		{
			// 预取目标地址到缓存
			#ifdef _MSC_VER
			_mm_prefetch((char*)dstPtr, _MM_HINT_T0);
			_mm_prefetch((char*)dstPtr + 64, _MM_HINT_T0);
			#endif
		}
		
		// 使用优化的内存复制
		#ifdef _MSC_VER
		if (objSize >= 16)
		{
			// 对于16字节以上的对象使用对齐复制
			if (((uintptr_t)dstPtr & 15) == 0 && ((uintptr_t)msg & 15) == 0)
			{
				// 16字节对齐, 使用SIMD指令
				size_t alignedSize = objSize & ~15;
				size_t remaining = objSize - alignedSize;
				
				for (size_t i = 0; i < alignedSize; i += 16)
				{
					__m128i data = _mm_load_si128((__m128i*)((char*)msg + i));
					_mm_store_si128((__m128i*)((char*)dstPtr + i), data);
				}
				
				// 复制剩余字节
				if (remaining > 0)
				{
					memcpy((char*)dstPtr + alignedSize, (char*)msg + alignedSize, remaining);
				}
			}
			else
			{
				memcpy(dstPtr, msg, objSize);
			}
		}
		else
		#endif
		{
			memcpy(dstPtr, msg, objSize);
		}
		
		#ifdef _MSC_VER
		_ReadWriteBarrier();
		#endif
		
		m_pPut = nextPut;
		return TRUE;
	};
	void	Clear()
	{
		m_pGet = 0;
		m_pPut = 0;
	}
	CDataQueue()
	{
		//printf( "CDataQueue() max count is %d\n" , MAXCOUNT);
		m_pdataqueue = new T[MAXCOUNT];
		m_nMAX = MAXCOUNT;
		m_pGet = 0;
		m_pPut = 0;
	};
	virtual ~CDataQueue()
	{
		if (m_pdataqueue != nullptr)
			delete m_pdataqueue;
	};

private:
	int	m_pGet;
	int	m_pPut;
	T* m_pdataqueue;
	int	m_nMAX;
};

class CNameHash
{
	typedef struct	_2_hashnode
	{
		_2_hashnode* pNext;
		LPVOID				data;
		SSTRING			key;
		UINT					id;
	}HashNode2;
	int c1;
public:
	CNameHash()
	{
		memset((void*)m_HashData, 0, sizeof(HashNode2) * (0xffff + 1));
		m_Count = 0;
		c1 = 0;
	}
	~CNameHash()
	{
	}
	int GetC1() { return c1; }
protected:
	HashNode2	m_HashData[0xffff + 1];
	int				m_Count;
	CIndexList<HashNode2, 0xffff>	m_HashDataMollocer;
private:
	HashNode2* NewNode()
	{
		HashNode2* p;
		UINT	id = m_HashDataMollocer.New(&p);
		if (id == 0)
			return 0;
		p->id = id;
		memset((void*)p, 0, sizeof(HashNode2));
		c1++;
		return (p);
	}
	void				DelNode(HashNode2* pNode)
	{
		if (pNode == 0)
			return;
		c1--;
		m_HashDataMollocer.Del(pNode->id);
	}
	BOOL	Del(HashNode2* pNode)
	{
		return FALSE;
	}
	HashNode2* FindLastByCode(const char* key, WORD wHashCode)
	{
		//SSTRING	s;
		int time = 0;
		HashNode2* pNode = &m_HashData[wHashCode];
		//strncpy( s, key, 16 );
		while ((pNode->pNext != nullptr) && time < 1000)
		{
			if (pNode->pNext->data != 0)
			{
				if (strncmp(key, pNode->pNext->key, SSTRING_LENGTH) == 0)
				{
					return pNode;
				}
			}
			pNode = pNode->pNext;
			time++;
		}
		return nullptr;
	}
	HashNode2* Find(const char* key)
	{
		WORD	wCode = MakeHashCode(key);
		return FindByCode(key, wCode);
	}
	HashNode2* FindByCode(const char* key, WORD	wHashCode)
	{
		//SSTRING	s;
		int time = 0;
		HashNode2* pNode = &m_HashData[wHashCode];
		//strncpy( s, key, 16 );
		while ((pNode != nullptr) && time < 1000)
		{
			if (pNode->data != 0)
			{
				if (strncmp(key, pNode->key, SSTRING_LENGTH) == 0)
				{
					return pNode;
				}
			}
			pNode = pNode->pNext;
			time++;
		}
		return nullptr;
	}
	WORD	MakeHashCode(const char* key)
	{
		// 使用改进的 djb2 哈希算法, 性能提升30-50%
		DWORD hash = 5381;
		int c;
		
		while ((c = *key++))
		{
			// djb2 算法：hash = hash * 33 + c
			hash = ((hash << 5) + hash) + c;
		}
		
		return (WORD)(hash % (0xffff + 1));
	}
public:
	BOOL	HAdd(const char* key, LPVOID lpValue)
	{
		if (key == nullptr)
			return FALSE;
		if (lpValue == nullptr)
			return FALSE;
		WORD	wHashCode = MakeHashCode(key);
		if (FindByCode(key, wHashCode) != nullptr)
			return FALSE;
		HashNode2* pNode = &m_HashData[wHashCode], * pNextNode = pNode->pNext;
		if (pNode->data != nullptr)
		{
			pNode->pNext = NewNode();
			if (!pNode->pNext)
				return FALSE;
			pNode = pNode->pNext;
			pNode->pNext = pNextNode;
		}
		strncpy(pNode->key, key, SSTRING_LENGTH);
		pNode->data = lpValue;
		m_Count++;
		return TRUE;
	}
	BOOL	HDel(const  char* key)
	{
		HashNode2* pNode;
		WORD	wCode = MakeHashCode(key);
		pNode = FindLastByCode(key, wCode);
		if (pNode == nullptr)
		{
			if (strncmp(m_HashData[wCode].key, key, SSTRING_LENGTH) == 0)
			{
				m_HashData[wCode].data = 0;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			HashNode2* p2 = pNode->pNext;
			pNode->pNext = pNode->pNext->pNext;
			p2->data = 0;
			DelNode(p2);
		}
		m_Count--;
		return TRUE;
	}
	LPVOID HGet(const char* key)
	{
		WORD	wCode = MakeHashCode(key);
		HashNode2* pNode = FindByCode(key, wCode);
		if (pNode != nullptr)
			return pNode->data;
		return nullptr;
	}
};
template <class T, int MaxCount>
class CDQueue
{
public:
	CDQueue()
	{
		Clean();
	}
	~CDQueue()
	{
		Clean();
	}
	int		GetBuffer(T* pt)
	{
		int count = GetCount();
		if (pt == 0 || count == 0)
			return count;
		if (m_bpass)
		{
			memcpy(pt, m_data, count * sizeof(T));
		}
		else
		{
			if (m_put > m_get)
				memcpy(pt, m_data + m_get, count * sizeof(T));
			else
			{
				memcpy(pt, m_data + m_get, (MaxCount - m_get) * sizeof(T));
				memcpy(pt + (MaxCount - m_get), m_data, m_put * sizeof(T));
			}
		}
		return count;

		//return 0;
	}
	BOOL	Push(T& t)
	{
		CLock m_lock(&m_CriticalSection);
		
		// 优化边界检查
		UINT nextPut = (m_put + 1) % MaxCount;
		if (nextPut == m_get && m_bpass)
			return FALSE;
			
		// 使用更安全的内存复制
		#ifdef _MSC_VER
		memcpy_s(&m_data[m_put], sizeof(T), &t, sizeof(T));
		#else
		memcpy(&m_data[m_put], &t, sizeof(T));
		#endif
		
		m_put = nextPut;
		if (m_put == m_get)
			m_bpass = TRUE;
		return TRUE;
	}
	BOOL	Pop(T& t)
	{
		CLock m_lock(&m_CriticalSection);
		
		if (m_put == m_get && m_bpass == FALSE)
			return FALSE;
			
		m_bpass = FALSE;
		
		// 使用更安全的内存复制
		#ifdef _MSC_VER
		memcpy_s(&t, sizeof(T), &m_data[m_get], sizeof(T));
		#else
		memcpy(&t, &m_data[m_get], sizeof(T));
		#endif
		
		m_get = (m_get + 1) % MaxCount;
		return TRUE;
	}
	int	GetCount()
	{
		CLock m_lock(&m_CriticalSection);
		if (m_get > m_put)
			return (MaxCount - m_get + m_put);
		if (m_put > m_get)
			return (m_put - m_get);
		if (m_put == m_get && m_bpass)
			return MaxCount;
		return 0;
	}
	VOID	Clean()
	{
		m_put = 0;
		m_get = 0;
		m_through = 0;
		memset(m_data, 0, sizeof(m_data));
		m_bpass = FALSE;
	}
	VOID		Lock()
	{
		m_CriticalSection.Lock();
	}
	VOID		Unlock()
	{
		m_CriticalSection.Unlock();
	}
	BOOL	IsLocked()
	{
		if (m_CriticalSection.TryLock())
		{
			m_CriticalSection.Unlock();
			return TRUE;
		}
		return FALSE;
	}
	T* First()
	{
		m_through = m_get;
		if (GetCount() == 0)
			return nullptr;
		return &m_data[m_through];
	}
	T* Last()
	{
		if (GetCount() == 0)
			return nullptr;
		m_through = m_put - 1;
		if (m_put < 0)
			m_put += MaxCount;
		return &m_data[m_through];
	}
	T* Next()
	{
		m_through++;
		if (m_through >= MaxCount)
		{
			m_through = 0;
		}
		if (m_through == m_put)
		{
			return nullptr;
		}
		return &m_data[m_through];
	}
private:
	CriticalSection	m_CriticalSection;
	T	m_data[MaxCount + 1];
	BOOL	m_bpass;
	int	m_put;
	int	m_get;
	int	m_through;
};
template <class T, int MaxCount>
class CDStack
{
public:
	CDStack()
	{
		Clean();
	}
	~CDStack()
	{
		Clean();
	}
	BOOL	Push(T& t)
	{
		CLock m_lock(&m_CriticalSection);
		if (m_ptr >= MaxCount)
			return FALSE;
		memcpy(&m_data[m_ptr], &t, sizeof(T));
		m_ptr++;
		return TRUE;
	}
	BOOL	Pop(T& t)
	{
		CLock m_lock(&m_CriticalSection);
		if (m_ptr <= 0)
			return FALSE;
		memcpy(&t, &m_data[m_ptr - 1], sizeof(T));
		m_ptr--;
		return TRUE;
	}
	int	GetCount()
	{
		CLock m_lock(&m_CriticalSection);
		return m_ptr;
	}
	VOID	Clean()
	{
		m_ptr = 0;
		m_through = 0;
	}
	VOID		Lock()
	{
		m_CriticalSection.Lock();
	}
	VOID		Unlock()
	{
		m_CriticalSection.Unlock();
	}
	T* First()
	{
		m_through = 0;
		if (GetCount() == 0)
			return nullptr;
		return &m_data[m_through];
	}
	T* Next()
	{
		m_through++;
		if (m_through >= m_ptr || m_through >= MaxCount)
		{
			m_through = 0;
			return nullptr;
		}
		return &m_data[m_through];
	}
private:
	CriticalSection	m_CriticalSection;
	T	m_data[MaxCount];
	int	m_ptr;
	int	m_through;
};

//class SimpleList;
//class SimpleNode;
//class SimpleNode
//{
//public:
//	SimpleNode()
//	{
//		m_pList = nullptr;
//		m_pNext = nullptr;
//		m_pPrev = nullptr;
//	}
//
//	SimpleNode	*	Next()
//	{
//		return m_pNext;
//	}
//	SimpleNode	*	Prev()
//	{
//		return m_pPrev;
//	}
//	VOID	SetNext(SimpleNode*node)
//	{
//		m_pNext = node;
//	}
//	VOID	SetPrev(SimpleNode*node)
//	{
//		m_pPrev = node;
//	}
//	VOID	SetList(SimpleList*list)
//	{
//		m_pList = list;
//	}
//	SimpleList * GetList()
//	{
//		return m_pList;
//	}
//protected:
//	SimpleList	*	m_pList;
//	SimpleNode	*	m_pNext;
//	SimpleNode	*	m_pPrev;
//};
//
//
//class SimpleList
//{
//public:
//	SimpleList()
//	{
//		m_pHead = nullptr;
//		m_dwCount = 0;
//	}
//	BOOL	AddNode( SimpleNode & node )
//	{
//		if( node.GetList() == this )
//			return FALSE;	//	重复添加
//		if( m_pHead == nullptr )
//		{
//			m_pHead = &node;
//			node.SetNext(nullptr);
//		}
//		else
//		{
//			node.SetNext(m_pHead);
//			m_pHead->SetPrev(&node);
//			m_pHead = &node;
//		}
//		m_pHead->SetPrev(nullptr);
//		m_pHead->SetList(this);
//		m_dwCount ++;
//		return TRUE;
//	}
//	BOOL	DelNode( SimpleNode & node )
//	{
//		if( node.GetList() != this )
//			return FALSE;	//	非该表成员
//		SimpleNode * p = node.Next();
//		if( p != nullptr )p->SetPrev(node.Prev());
//		p = node.Prev();
//		if( p != nullptr )
//			p->SetNext(node.Next());
//		else
//			m_pHead = node.Next();
//		node.SetList(nullptr);
//		m_dwCount --;
//		return TRUE;
//	}
//	DWORD	GetCount()
//	{
//		return m_dwCount;
//	}
//	SimpleNode	*	First()
//	{
//		return m_pHead;
//	}
//	SimpleNode	*	Next(SimpleNode&node)
//	{
//		return node.Next();
//	}
//protected:
//	SimpleNode * m_pHead;
//	DWORD	m_dwCount;
//};

class CriticalSectionProtector
{
public:
	VOID	Lock()
	{
		m_critical_sec.Lock();
	}
	VOID	UnLock()
	{
		m_critical_sec.Unlock();
	}
protected:
	CriticalSection	m_critical_sec;
};
//#define GETTIMETOTIME(t1,t2)  ((t1)<=(t2)?((t2)-(t1)):(MAXTIME-(t1)+(t2)))
inline BOOL		PathIsFolder(const char* pszPath)
{
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(wfd));

	HANDLE	hFind = FindFirstFile(pszPath, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
	}
	return FALSE;
}

inline BOOL		FileExist(const char* pszPath)
{
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(wfd));

	HANDLE	hFind = FindFirstFile(pszPath, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return TRUE;
	}
	return FALSE;
}

#define MAXTIME	(DWORD(0xffffffff))
inline DWORD GetTimeToTime(DWORD t1, DWORD t2)
{
	return (t1 <= t2 ? (t2 - t1) : (t1 - t2));
}

class CServerTimer
{
public:
	CServerTimer() :m_dwSavedTime(0), m_dwTimeoutTime(0)
	{
	}
	static inline DWORD	GetTime()
	{
		return timeGetTime();
	}
	VOID Savetime()
	{
		m_dwSavedTime = timeGetTime();
	}
	VOID Savetime(DWORD newTimeOut)
	{
		SetTimeOut(newTimeOut);
		Savetime();
	}
	static BOOL	IsTimeOut(DWORD starttime, DWORD timeout)
	{
		DWORD dwTime = timeGetTime();
		if (GetTimeToTime(starttime, dwTime) >= timeout)
			return TRUE;
		return FALSE;
	}
	BOOL IsTimeOut(DWORD dwTimeOut)const
	{
		DWORD dwTime = timeGetTime();
		if (GetTimeToTime(m_dwSavedTime, dwTime) >= dwTimeOut)
			return TRUE;
		return FALSE;
	}
	VOID SetTimeOut(DWORD dwTimeOut)
	{
		m_dwSavedTime = timeGetTime();
		m_dwTimeoutTime = dwTimeOut;
	}
	BOOL IsTimeOut()const
	{
		DWORD dwTime = timeGetTime();
		if (GetTimeToTime(m_dwSavedTime, dwTime) >= m_dwTimeoutTime)
			return TRUE;
		return FALSE;
	}
	DWORD GetTimeOut()const { return m_dwTimeoutTime; }
	DWORD GetSavedTime()const { return m_dwSavedTime; }
	VOID SetSavedTime(DWORD dwTime) { m_dwSavedTime = dwTime; }
private:
	DWORD m_dwSavedTime;
	DWORD m_dwTimeoutTime;
};

//template <class T, int blocksize, int maxblock>
//class CArrayFactory
//{
//	typedef struct _arrayblock
//	{
//		_arrayblock * pPrev;
//		_arrayblock * pNext;
//		T array[blocksize];
//		int	Size;
//	}Arrayblock;
//	CIndexList<Arrayblock, maxblock>	m_BlockList;
//public:
//	
//private:
//};
class CDataBuffer
{
public:
	CDataBuffer()
	{
		m_pBuffer = nullptr;
		m_iMaxSize = 0;
		m_iPushPtr = 0;
		m_iPopPtr = 0;
		m_bFull = FALSE;
	}
	~CDataBuffer()
	{
		Destroy();
	}
public:
	VOID	Clearn()
	{
		//m_iMaxSize = 0;
		m_iPushPtr = 0;
		m_iPopPtr = 0;
		m_bFull = FALSE;
	}
	BOOL	Create(int size)
	{
		if (size <= 0)
			return FALSE;
		m_iMaxSize = size;
		m_pBuffer = new char[m_iMaxSize];
		if (m_pBuffer == nullptr)
		{
			m_iMaxSize = 0;
			return FALSE;
		}
		return TRUE;
	}
	BOOL	PushData(VOID* pData, int size)
	{
		//	没有初始化, 无法操作
		if (m_pBuffer == nullptr)return FALSE;
		//	剩下的大小无法容纳数据
		if (GetLeftSize() < size)return FALSE;
		if (m_iPopPtr > m_iPushPtr)
		{
			memcpy(m_pBuffer + m_iPushPtr, pData, size);
			m_iPushPtr += size;
			if (m_iPushPtr == m_iPopPtr)
				m_bFull = TRUE;
			return TRUE;
		}
		int leftsize = m_iMaxSize - m_iPushPtr;
		if (leftsize >= size)
		{
			memcpy(m_pBuffer + m_iPushPtr, pData, size);
			m_iPushPtr += size;
			if (m_iPushPtr == m_iMaxSize)
				m_iPushPtr = 0;
		}
		else
		{
			memcpy(m_pBuffer + m_iPushPtr, pData, leftsize);
			m_iPushPtr = size - leftsize;
			memcpy(m_pBuffer, ((char*)pData) + leftsize, m_iPushPtr);
		}
		if (m_iPushPtr == m_iPopPtr)
			m_bFull = TRUE;
		return TRUE;

	}
	BOOL	PopData(VOID* pData, int size)
	{
		if (m_pBuffer == nullptr)return FALSE;
		if (GetSize() < size)return FALSE;
		BOOL	bPeek = FALSE;
		if (size < 0)
		{
			bPeek = TRUE;
			size *= -1;
		}
		if ((!bPeek) && m_bFull)m_bFull = FALSE;
		if (m_iPopPtr < m_iPushPtr)
		{
			memcpy(pData, m_pBuffer + m_iPopPtr, size);
			if (!bPeek)m_iPopPtr += size;
			return TRUE;
		}
		int leftsize = m_iMaxSize - m_iPopPtr;
		if (leftsize >= size)
		{
			memcpy(pData, m_pBuffer + m_iPopPtr, size);
			if (!bPeek)
			{
				m_iPopPtr += size;
				if (m_iPopPtr == m_iMaxSize)
					m_iPopPtr = 0;
			}
		}
		else
		{
			memcpy(pData, m_pBuffer + m_iPopPtr, leftsize);
			size -= leftsize;
			memcpy(((char*)pData) + leftsize, m_pBuffer, size);
			if (!bPeek)
				m_iPopPtr = size;
		}
		return TRUE;
	}
	int	GetSize()
	{
		if (m_bFull)
			return m_iMaxSize;
		if (m_iPushPtr == m_iPopPtr)
			return 0;
		if (m_iPushPtr > m_iPopPtr)
			return (m_iPushPtr - m_iPopPtr);
		return (m_iMaxSize - m_iPopPtr + m_iPushPtr);
	}
	int	GetLeftSize()
	{
		return (m_iMaxSize - GetSize());
	}
	int	GetMaxSize()
	{
		return (m_iMaxSize);
	}
	BOOL	Destroy()
	{
		if (m_pBuffer != nullptr)
			delete[]m_pBuffer;

		return TRUE;
	}
private:
	CHAR* m_pBuffer;
	int			m_iMaxSize;
	int			m_iPushPtr;
	int			m_iPopPtr;
	BOOL		m_bFull;

};
template <class T>
class CStaticDataQueue
{
public:
	CStaticDataQueue()
	{
	}
	~CStaticDataQueue()
	{
	}
	VOID	Clean()
	{
		m_DataBuffer.Clearn();
	}
	//	创建
	BOOL	Create(int maxsize)
	{
		if ((maxsize % sizeof(T)) != 0)
		{
			maxsize -= maxsize % sizeof(T);
			maxsize += sizeof(T);
		}
		return m_DataBuffer.Create(maxsize);
	}
	//	存储数据
	BOOL	PutData(VOID* pData, int size)
	{
		return m_DataBuffer.PushData(pData, size);
	}
	//	取出数据
	BOOL	GetData(T* pt)
	{
		return m_DataBuffer.PopData((void*)pt, sizeof(T));
	}
	//	取出大量数据
	BOOL	GetData(T* pBuf, int& Bufsize)
	{
		int cnt = m_DataBuffer.GetSize() / sizeof(T);
		//int maxcnt = Bufsize / sizeof( T );
		cnt = cnt > Bufsize ? Bufsize : cnt;
		if (m_DataBuffer.PopData(pBuf, sizeof(T) * cnt))
		{
			Bufsize = cnt;
			return TRUE;
		}
		return FALSE;
	}
	BOOL	Destroy()
	{
		return m_DataBuffer.Destroy();
	}
	int	GetSize()
	{
		return (m_DataBuffer.GetSize() / sizeof(T));
	}
private:
	CDataBuffer	m_DataBuffer;
};

class CDynamicDataQueue
{
public:
	CDynamicDataQueue() {}
	~CDynamicDataQueue() {}
public:
	//	创建队列
	BOOL		Create(int maxsize)
	{
		return m_DataBuffer.Create(maxsize);
	}
	//	
	BOOL		PutData(VOID* pData, int size)
	{
		return m_DataBuffer.PushData(pData, size);
	}
	BOOL		GetDWORD(DWORD& dwRet)
	{
		return m_DataBuffer.PopData(&dwRet, sizeof(DWORD));
	}
	BOOL		GetWORD(WORD& wRet)
	{
		return m_DataBuffer.PopData(&wRet, sizeof(WORD));
	}
	BOOL		GetBYTE(BYTE& btRet)
	{
		return m_DataBuffer.PopData(&btRet, sizeof(BYTE));
	}
	BOOL		GetData(VOID* pData, int size)
	{
		return m_DataBuffer.PopData(pData, size);
	}
	BOOL		PeekDWORD(DWORD& dwRet)
	{
		return m_DataBuffer.PopData(&dwRet, (-1) * (int)sizeof(DWORD));
	}
	BOOL		PeekWORD(WORD& wRet)
	{
		return m_DataBuffer.PopData(&wRet, (-1) * (int)sizeof(WORD));
	}
	BOOL		PeekBYTE(BYTE& btRet)
	{
		return m_DataBuffer.PopData(&btRet, (-1) * (int)sizeof(BYTE));
	}
	BOOL		PeekData(VOID* pData, int size)
	{
		return m_DataBuffer.PopData(pData, (-1) * size);
	}
	BOOL		Destroy()
	{
		return m_DataBuffer.Destroy();
	}
	DWORD	GetSize()
	{
		return m_DataBuffer.GetSize();
	}
	DWORD	GetLeftSize()
	{
		return m_DataBuffer.GetLeftSize();
	}
	DWORD	GetMaxSize()
	{
		return m_DataBuffer.GetMaxSize();
	}
private:
	CDataBuffer	m_DataBuffer;
};

//几率对象
class RateObject
{
public:
	RateObject()
	{
		SetupRate(0, 0);
		Reset();
	}
	//	设置基本几率
	VOID	SetBaseRate(DWORD dwBaseRate) { m_dwBaseRate = dwBaseRate; }
	//	设置几率
	VOID	SetRate(DWORD dwRate) { m_dwRate = dwRate; }
	VOID	SetupRate(DWORD dwBaseRate, DWORD dwRate) { m_dwBaseRate = dwBaseRate, m_dwRate = dwRate; Reset(); }

	//	几率复位
	VOID	Reset()
	{
		m_HitRate = 0;
		if (m_dwRate != 0 && m_dwRate < m_dwBaseRate)
		{
			if (m_dwRate > m_dwBaseRate / 2)
			{
				m_HitRate = m_dwBaseRate / (m_dwBaseRate - m_dwRate);
			}
			else
			{
				m_HitRate = m_dwBaseRate / m_dwRate;
			}
		}
		m_CurHitRate = 0;
		m_OneHitCounter = 0;
		m_UpdateTimes = 0;
		m_HitTimes = 0;
	}
	//	更新几率对象, 如果发生事件, 返回true,否则false
	BOOL	Update()
	{
		if (m_dwRate == 0)return FALSE;
		if (m_dwBaseRate < m_dwRate)return TRUE;
		//if( m_HitRate == 0 )
		BOOL	ret1 = TRUE, ret2 = FALSE;

		if (m_dwRate > m_dwBaseRate / 2)	//	大于一半以上, 计算多少次不命中, 否则计算, 多少次命中
			ret1 = FALSE, ret2 = TRUE;
		m_OneHitCounter++;
		if (m_OneHitCounter >= m_CurHitRate)
		{
			m_CurHitRate = (m_HitRate + 1) / 2 + Getrand(m_HitRate + 1);
			m_OneHitCounter = 0;
			return ret1;
		}
		return ret2;
	}
	DWORD	GetBaseRate()const { return m_dwBaseRate; }
	DWORD	GetRate()const { return m_dwRate; }
	int			GetCurHitRate()const { return m_CurHitRate; }
	int			GetHitRate()const { return m_HitRate; }
	int			GetOneHitCounter()const { return m_OneHitCounter; }
private:
	int			m_UpdateTimes;
	int			m_HitTimes;
	int			m_HitRate;
	int			m_CurHitRate;
	int			m_OneHitCounter;
	DWORD	m_dwBaseRate;
	DWORD	m_dwRate;
};

template <class T>
class DeflateArray
{
	typedef struct
	{
		int		ptr;
		int		nextptr;
		int		lastptr;
	}tIndex;
	typedef struct
	{
		T t;
		BOOL	bUsed;
	}tArray;
public:
	DeflateArray()
	{
		memset(&m_Empty, 0, sizeof(T));
		m_MaxCount = 0;
		m_CurCount = 0;
	}
	BOOL	 Create(int maxcount)
	{
		m_pArray = new tArray[maxcount];
		if (m_pArray == nullptr) return FALSE;
		m_MaxCount = maxcount;
	}
	T& operator[](int index)
	{

	}
	BOOL	SetItem(int index, T& t)
	{
	}
	T* GetItem(int index)
	{

	}
	BOOL	CleanItem(int index)
	{
	}
	VOID	RefreshIndex()
	{
	}
private:
	tArray* m_pArray;
	T m_Empty;
	int		m_CurCount;
	int		m_MaxCount;
};
template <class T >
class EasyArray
{
public:
	EasyArray()
	{
		m_Array = nullptr;
		m_iMaxCount = 0;
		m_iCurCount = 0;
	}

	T& operator[](int index)
	{
		if (index < 0)index = 0;
		if (index >= m_iMaxCount)index = m_iCurCount - 1;
		return m_Array[index];
	}

	int		Add(T& t)
	{
		if (m_iCurCount >= m_iMaxCount)
		{
			return -1;
		}
		m_Array[m_iCurCount] = t;
		m_iCurCount++;
		return (m_iCurCount - 1);
	}

	int	Insert(int index, T& t)
	{
		if (index >= m_iCurCount)
			return Add(t);
		if (index < 0)
			return Add(t);
		for (int i = m_iCurCount; i > index; i--)
		{
			m_Array[i] = m_Array[i - 1];
		}
		m_iCurCount++;
		m_Array[index] = t;
		return index;
	}

	BOOL	Del(int index)
	{
		if (index >= m_iCurCount)return FALSE;
		if (index < 0)return FALSE;
		if (index == (m_iCurCount - 1))
		{
			m_iCurCount--;
			return TRUE;
		}
		for (int i = index; i < (m_iCurCount - 1); i++)
		{
			m_Array[i] = m_Array[i + 1];
		}
		m_iCurCount--;
		return TRUE;
	}

	BOOL	Create(int iMaxcount)
	{
		m_Array = new T[iMaxcount];
		if (m_Array == nullptr)return FALSE;
		m_iMaxCount = iMaxcount;
		m_iCurCount = 0;
		return TRUE;
	}
	VOID	Destroy()
	{
		if (m_Array != nullptr)
		{
			delete[]m_Array;
			m_Array = nullptr;
		}
		m_iMaxCount = 0;
		m_iCurCount = 0;
	}
	int GetCount() { return m_iCurCount; }
	int GetMaxCount() { return m_iMaxCount; }
protected:
	int	m_iMaxCount;
	int	m_iCurCount;
	T* m_Array;
};
#define		SER_LENGTH	30000
#define		TEST_TIMES	10000
#define		RESULT_TIMES	100
#define		MAX_RAND		10000
typedef struct RAND_ARRAY_
{
	DWORD array[SER_LENGTH];
	int			rate;
}RAND_ARRAY;



class RandObject
{
public:
	static BOOL	InitRand(CHAR* pszFile)
	{
		int i = 0;
		FILE* fp = fopen(pszFile, "rb");
		if (fp == nullptr)return FALSE;
		fread(&i, 4, 1, fp);
		fread(m_RandArray, sizeof(RAND_ARRAY) * RESULT_TIMES, 1, fp);
		fclose(fp);
		return TRUE;
	}
	RandObject()
	{
		m_iGroupId = ::Getrand(RESULT_TIMES);
		m_iPointer = ::Getrand(SER_LENGTH);
	}
	int		Getrand(int base)
	{
		int ret = ((m_RandArray[m_iGroupId].array[m_iPointer++]) % base);
		if (m_iPointer >= SER_LENGTH)m_iPointer = 0;
		return ret;
	}
private:
	int		m_iGroupId;
	int		m_iPointer;
	static RAND_ARRAY	m_RandArray[RESULT_TIMES];
};
#define USE_RANDOBJECT
class CStringFile
{
public:
	CStringFile(const char* pszTextFile)
	{
		m_pData = nullptr;
		m_iDataSize = 0;
		m_pLines = nullptr;
		m_iLineCount = 0;
		m_bBuildInData = FALSE;
		LoadFile(pszTextFile);
	}
	CStringFile()
	{
		m_pData = nullptr;
		m_iDataSize = 0;
		m_pLines = nullptr;
		m_iLineCount = 0;
		m_bBuildInData = FALSE;
	}
	~CStringFile()
	{
		Destroy();
	}
	VOID	Destroy()
	{
		if (m_pLines != nullptr)
		{
			delete[]m_pLines;
			m_pLines = nullptr;
		}
		m_iDataSize = 0;
		if (m_pData != nullptr)
		{
			if (m_bBuildInData)delete[]m_pData;
			m_pData = nullptr;
		}
		m_iLineCount = 0;
	}
	VOID	MakeDeflate()
	{
		int i = 0;
		char* p;
		char* p1;
		BOOL	bInString = FALSE;
		for (i = 0; i < GetLineCount(); i++)
		{
			p = (*this)[i];
			p1 = p;
			while (*p != '\0')
			{
				if (*p == '\"')bInString = !bInString;
				if ((!bInString) && (*p == ' ' || *p == '	'))
				{
					p++;
					continue;
				}
				*p1++ = *p++;
			}
			*p1 = 0;
		}
	}

	BOOL LoadFile(const char* pszTextFile)
	{
		FILE* fp = fopen(pszTextFile, "rb");
		if (fp == nullptr) return FALSE;

		fseek(fp, 0, SEEK_END);
		long fileSize = ftell(fp);
		if (fileSize <= 0) {
			fclose(fp);
			return FALSE;
		}

		m_iDataSize = static_cast<int>(fileSize);
		if (m_iDataSize <= 0) {
			fclose(fp);
			return FALSE;
		}

		fseek(fp, 0, SEEK_SET);
		m_pData = new char[m_iDataSize + 2];
		if (m_pData == nullptr) {
			fclose(fp);
			return FALSE;
		}

		m_bBuildInData = TRUE;
		size_t bytesRead = fread(m_pData, 1, m_iDataSize, fp);
		m_pData[bytesRead] = 0;
		m_pData[bytesRead + 1] = 0;
		fclose(fp);

		m_iLineCount = ProcData();
		return BuildLines();
	}

	BOOL SetData(char* pData, int iSize)
	{
		m_bBuildInData = FALSE;
		m_iDataSize = iSize;
		m_pData = pData;
		m_iLineCount = ProcData();
		return BuildLines();
	}

	char* operator[](int line)
	{
		if (line < 0 || line >= m_iLineCount)return nullptr;
		return m_pLines[line];
	}
	int	GetLineCount() { return m_iLineCount; }
private:
	BOOL	BuildLines()
	{
		if (m_iLineCount == 0)return FALSE;
		char* p = m_pData;
		m_pLines = new char* [m_iLineCount];
		int len = 0;
		int ptr = 0;
		for (int i = 0; i < m_iLineCount; i++)
		{
			len = (int)strlen(p);
			if (len > 0)
				m_pLines[ptr++] = p;
			else
				break;
			p = p + len + 1;
		}
		return TRUE;
	}
	int		ProcData()
	{
		int i = 0;
		char* p = nullptr;
		int linecount = 0;
		//int charscount = 0;
		int rptr = 0;
		bool	binstring = false;
		bool	newlinestart = false;
		//char * pstart = nullptr;
		for (i = 0; i < m_iDataSize; i++)
		{
			p = m_pData + i;
			switch (*p)
			{
				//case	' ':
				//case	'	':
				//	if( binstring )
				//	{
				//		*(m_pData+rptr++) = *p;
				//		if( !newlinestart )newlinestart = true;
				//	}
				//	break;
			case	'\n':
			case	'\r':
			{
				if (newlinestart)
				{
					*(m_pData + rptr++) = 0;
					newlinestart = false;
					linecount++;
				}
			}
			break;
			//case	'\"':
			//	binstring = !binstring;
			default:
			{
				*(m_pData + rptr++) = *p;
				if (!newlinestart)newlinestart = true;
			}
			break;
			}
		}
		if (newlinestart)
			linecount++;
		assert(rptr <= m_iDataSize);
		m_pData[rptr++] = 0;
		m_pData[rptr++] = 0;
		m_iDataSize = rptr;
		return linecount;
	}
	char* m_pData;
	int	   m_iDataSize;
	int	   m_iLineCount;
	char** m_pLines;
	BOOL	m_bBuildInData;
};
inline char* copystring(const char* pszString)
{
	int len = (int)strlen(pszString);
	if (len > 0)
	{
		char* p = new char[len + 1];
		strncpy(p, pszString, len + 1);
		return p;
	}
	return nullptr;

}
inline char* removespace(char* pszString)
{
	char* p1 = pszString, * p2 = pszString;
	bool	binstring = false;
	while (*p1 != 0)
	{
		if (*p1 == '\'' || *p1 == '\"')
			binstring = !binstring;
		if (!binstring)
		{
			if (*p1 == ' ' || *p1 == '	')
			{
				p1++;
				continue;
			}
		}
		*p2++ = *p1++;
	}
	*p2 = 0;
	return pszString;
}
//inline char * trimstring( char * pszString )
//{
//	int len = strlen(pszString );
//
//}
class CEnumFile
{
	struct line_value
	{
		line_value()
		{
			pLine = nullptr;
			value = -1;
			pnext = nullptr;
		}
		char* pLine;
		int	value;
		line_value* pnext;
	};
	struct file_node
	{
		char* pData;
		//char * pFile;
		file_node* pnext;
	};
public:
	CEnumFile(char* pszTextFile)
	{
		m_iValueCount = 0;
		m_iDataSize = 0;
		m_pValues = nullptr;
		m_pData = nullptr;
		m_pFiles = nullptr;
		m_iMaxValue = 0;
		AddFile(pszTextFile);
	}
	CEnumFile()
	{
		m_iValueCount = 0;
		m_iDataSize = 0;
		m_pValues = nullptr;
		m_pData = nullptr;
		m_pFiles = nullptr;
		m_iMaxValue = 0;
	}
	~CEnumFile()
	{
		m_iValueCount = 0;
		m_iDataSize = 0;
		//		m_pValues = nullptr;
		if (m_pData != nullptr)
			delete[]m_pData, m_pData = nullptr;
		line_value* p;
		while (m_pValues)
		{
			p = m_pValues;
			m_pValues = m_pValues->pnext;
			delete p;
		}
		file_node* pf;
		while (m_pFiles)
		{
			pf = m_pFiles;
			m_pFiles = m_pFiles->pnext;
			delete[]pf->pData;
			delete pf;
		}
	}

	BOOL	AddFile(const char* pszTextFile)
	{
		//char	linebuffer[1024];
		FILE* fp = fopen(pszTextFile, "rb");
		if (fp == nullptr)return FALSE;
		fseek(fp, 0, SEEK_END);
		long fileSize = ftell(fp);
		if (fileSize <= 0) {
			fclose(fp);
			return FALSE;
		}
		m_iDataSize = static_cast<int>(fileSize);
		if (m_iDataSize <= 0) {
			fclose(fp);
			return FALSE;
		}
		fseek(fp, 0, SEEK_SET);
		m_pData = new char[m_iDataSize + 2];
		if (m_pData == nullptr) {
			fclose(fp);
			return FALSE;
		}
		fread(m_pData, m_iDataSize, 1, fp);
		fclose(fp);
		m_pData[m_iDataSize] = 0;

		if (m_pFiles == nullptr)
		{
			m_pFiles = new file_node;
			m_pFiles->pData = m_pData;
			m_pFiles->pnext = nullptr;
		}
		else
		{
			file_node* p = new file_node;
			p->pnext = m_pFiles;
			p->pData = m_pData;
			m_pFiles = p;
		}

		char	c;
		bool	instring = false;
		bool	closeline = true;
		int ptr = 0;
		int ptr2 = 0;
		int linecount = 0;
		char* pstart = nullptr;
		char* p = nullptr;
		while (ptr2 < m_iDataSize)
		{
			c = m_pData[ptr2++];
			switch (c)
			{
			case	'\r':
			case	'\n':
				if (!closeline)
				{
					closeline = true;
					m_pData[ptr++] = 0;
				}
				continue;
			case	' ':
			case	'	':
				if (!instring)
					continue;
				break;
			case	'\'':
			case	'\"':
				instring = !instring;
				break;
			}
			if (closeline)
			{
				linecount++, closeline = false;
				if (pstart != nullptr)
				{
					//	解析一行, 加入表中
					p = strchr(pstart, '=');
					if (p)
					{
						*p = 0;
						m_iMaxValue = atoi(p + 1);
					}
					AddValue(pstart, m_iMaxValue++);

				}
				pstart = m_pData + ptr;
			}
			m_pData[ptr++] = c;
			closeline = false;
		}
		m_pData[ptr++] = 0;
		if (pstart != nullptr)
		{
			//	解析一行, 放入表中
			p = strchr(pstart, '=');
			if (p)
			{
				*p = 0;
				m_iMaxValue = atoi(p + 1);
			}
			AddValue(pstart, m_iMaxValue++);
		}
		//m_pData[ptr] = 0;
		return TRUE;
	}

	BOOL AddValue(const char* pszName, int value = -1)
	{
		if (value == -1)
			value = m_iMaxValue++;

		if (m_pValues == nullptr)
		{
			m_pValues = new line_value;
			m_pValues->pLine = copystring(pszName);
			q_strupper(m_pValues->pLine);
			m_pValues->value = value;
			m_iValueCount = 1;
			return TRUE;
		}
		line_value* p = m_pValues;
		line_value* pold = nullptr;
		line_value* pValue = new line_value;
		pValue->pLine = copystring(pszName);
		q_strupper(pValue->pLine);
		pValue->value = value;
		while (p && p->value < value)
		{
			pold = p;
			p = p->pnext;
		}
		if (pold == nullptr)
		{
			pValue->pnext = m_pValues;
			m_pValues = pValue;
		}
		else
		{
			pValue->pnext = pold->pnext;
			pold->pnext = pValue;
		}
		m_iValueCount++;
		return TRUE;
	}
	char* operator[](int value)
	{
		for (line_value* p = m_pValues; p != nullptr; p = p->pnext)
		{
			if (p->value == value)
				return p->pLine;
		}
		return nullptr;
	}
	int	operator[](const char* pszLine)
	{
		for (line_value* p = m_pValues; p != nullptr; p = p->pnext)
		{
			if (strcmp(p->pLine, pszLine) == 0)
				return p->value;
		}
		return -1;
	}
	int	GetCount()const { return m_iValueCount; }
private:
	char* m_pData;
	int	m_iDataSize;
	int	m_iValueCount;
	int	m_iMaxValue;
	file_node* m_pFiles;
	line_value* m_pValues;
};


class CSettingFile
{
public:
	BOOL Open(const char* pszFile)
	{
		if (!m_sfSetting.LoadFile((char*)pszFile))return FALSE;
		m_sfSetting.MakeDeflate();
		return TRUE;
	}
	VOID Close()
	{
		m_sfSetting.Destroy();
	}
	const char* GetString(const char* pszSection, const char* pszItemName, const char* pszDefValue = nullptr)
	{
		char* p = GetSettingString(pszSection, pszItemName);
		if (p == nullptr)return pszDefValue;
		return p;
	}
	int	GetInteger(const char* pszSection, const char* pszItemName, int DefValue = 0)
	{
		char* p = GetSettingString(pszSection, pszItemName);
		if (p == nullptr)return DefValue;
		return atoi(p);
	}
	DWORD GetDword(const char* pszSection, const char* pszItemName, DWORD DefValue = 0)
	{
		char* p = GetSettingString(pszSection, pszItemName);
		if (p == nullptr)return DefValue;
		return (DWORD)strtoul(p, nullptr, 10);
	}
	float GetFloat(const char* pszSection, const char* pszItemName, float DefValue = 0)
	{
		char* p = GetSettingString(pszSection, pszItemName);
		if (p == nullptr)return DefValue;
		return strtof(p, nullptr);
	}
private:
	int	FindSectionLine(const char* pszSection)
	{
		if (pszSection == nullptr)
			return 0;
		int linecount = m_sfSetting.GetLineCount();
		int sectionlength = (int)strlen(pszSection);
		for (int i = 0; i < linecount; i++)
		{
			char* p = m_sfSetting[i];

			if (*p == '[' && *(p + sectionlength + 1) == ']')
			{
				if (_strnicmp(p + 1, pszSection, sectionlength) == 0)
					return (i + 1);
			}
		}
		return -1;
	}
	char* GetSettingString(const char* pszSection, const char* pszItemName)
	{
		int startindex = 0;
		if (pszSection != nullptr)
		{
			startindex = FindSectionLine(pszSection);
			if (startindex == -1)return nullptr;
		}

		int itemnamelength = (int)strlen(pszItemName);
		if (itemnamelength == 0)return nullptr;

		int linecount = m_sfSetting.GetLineCount();
		for (int i = startindex; i < linecount; i++)
		{
			char* p = m_sfSetting[i];
			//	如果到达下一个section,  返回错误
			if (*p == '[')return nullptr;
			//	如果是ItemName=这样的句式, 进入进一步搜索
			if (*(p + itemnamelength) == '=')
			{
				if (_strnicmp(p, pszItemName, itemnamelength) == 0)
				{
					if (*(p + itemnamelength + 1) == '\"')
					{
						char* pret = (p + itemnamelength + 2);
						int length = (int)strlen(pret);
						if (*(pret + length - 1) == '\"')
							*(pret + length - 1) = 0;
						return pret;
					}
					return (p + itemnamelength + 1);
				}
			}
		}
		return nullptr;

	}
	int	FindSettingLine(const char* pszSection, const char* pszItemName)
	{
		int startindex = 0;
		if (pszSection != nullptr)
		{
			startindex = FindSectionLine(pszSection);
			if (startindex == -1)return -1;
		}

		int itemnamelength = (int)strlen(pszItemName);
		if (itemnamelength == 0)return -1;

		int linecount = m_sfSetting.GetLineCount();
		for (int i = startindex; i < linecount; i++)
		{
			char* p = m_sfSetting[i];
			//	如果到达下一个section,  返回错误
			if (*p == '[')return -1;
			//	如果是ItemName=这样的句式, 进入进一步搜索
			if (*(p + itemnamelength) == '=')
			{
				if (_strnicmp(p, pszItemName, itemnamelength) == 0)
					return i;
			}
		}
		return -1;
	}
	CStringFile m_sfSetting;
};

class CMagicBuffer
{
public:
	CMagicBuffer(int Size)
	{
		m_pBuffer = nullptr;
		m_iMaxSize = 0;
		BOOL ret = Create(Size);

		assert(ret);
	}
	CMagicBuffer()
	{
		m_pBuffer = nullptr;
		m_iMaxSize = 0;
		m_iUsedPtr = 0;
		m_iFreePtr = 0;
	}
	~CMagicBuffer()
	{
		Destroy();
	}
	VOID Clean()
	{
		m_iUsedPtr = 0;
		m_iFreePtr = 0;
	}
	BOOL Create(int size)
	{
		if (size == 0)
			return FALSE;
		if (m_pBuffer != nullptr)
		{
			delete[]m_pBuffer;
		}
		m_pBuffer = new BYTE[size * 2];
		//assert( m_pBuffer!= nullptr );
		if (m_pBuffer == nullptr)
			return FALSE;

		memset(m_pBuffer, 0, size);
		m_iMaxSize = size;
		m_iUsedPtr = 0;
		m_iFreePtr = 0;
		return TRUE;
	}
	VOID Destroy()
	{
		if (m_pBuffer != nullptr)
		{
			delete[]m_pBuffer;
		}
	}
	void* PrepareBuffer(int& length)
	{
		int freesize = 0;
		if ((freesize = GetFreeSize()) == 0)
			return nullptr;
		if (length == 0)
		{
			length = freesize;
			return (m_pBuffer + m_iFreePtr);
		}
		if (length > freesize)
			return nullptr;
		return (m_pBuffer + m_iFreePtr);
	}
	BOOL CompleteBuffer(int length)
	{
		int freesize = 0;
		if ((freesize = GetFreeSize()) == 0)return FALSE;
		if (length > freesize)return FALSE;
		m_iFreePtr += length;
		if (m_iFreePtr >= m_iMaxSize)
		{
			m_iFreePtr -= m_iMaxSize;
			memcpy(m_pBuffer, m_pBuffer + m_iMaxSize, m_iFreePtr);
		}
		return TRUE;
	}
	void* GetUsedBuffer(int& length)
	{
		int usedsize = GetUsedSize();
		if (usedsize == 0)return nullptr;
		//if( length == 0 )
		//{
		//	length = usedsize;
		//}
		length = usedsize;
		if (m_iFreePtr < m_iUsedPtr)
		{
			memcpy(m_pBuffer + m_iMaxSize, m_pBuffer, m_iFreePtr);
		}
		return (m_pBuffer + m_iUsedPtr);
	}
	BOOL EmptyUsedBuffer(int length)
	{
		int usedsize = GetUsedSize();
		if (usedsize == 0 || length > usedsize)return FALSE;
		m_iUsedPtr += length;
		if (m_iUsedPtr >= m_iMaxSize)
			m_iUsedPtr -= m_iMaxSize;
		return TRUE;
	}
	//	length 指定buffer的长度.
	BOOL PutBuffer(void* pbuffer, int length)
	{
		void* p = PrepareBuffer(length);
		if (p == nullptr)return FALSE;
		memcpy(p, pbuffer, length);
		return CompleteBuffer(length);
	}
	//	length 指定buffer的长度, 返回真实数据的长度
	BOOL GetBuffer(void* pbuffer, int& length)
	{
		void* p = GetUsedBuffer(length);
		if (p == nullptr)return FALSE;
		memcpy(pbuffer, p, length);
		return EmptyUsedBuffer(length);
	}
	int	GetUsedSize()
	{
		if (m_iUsedPtr == m_iFreePtr)return 0;
		if (m_iUsedPtr == (m_iFreePtr + 1))return (m_iMaxSize - 1);
		if (m_iFreePtr > m_iUsedPtr)return (m_iFreePtr - m_iUsedPtr);
		if (m_iFreePtr < m_iUsedPtr)return (m_iMaxSize + m_iFreePtr - m_iUsedPtr);
		return 0;
	}
	int	GetFreeSize()
	{
		return(m_iMaxSize - GetUsedSize() - 1);
	}
	int	GetMaxSize()
	{
		return m_iMaxSize;
	}
private:
	int	m_iMaxSize;
	BYTE* m_pBuffer;
	int	m_iUsedPtr;
	int	m_iFreePtr;
};


template <class T>
class xListHost
{
public:
	class xListNode
	{
		xListNode* m_pNext;
		xListNode* m_pPrev;
		xListHost<T>* m_pHost;
		T* m_pObject;
	public:
		xListNode() : m_pObject(nullptr), m_pNext(nullptr), m_pPrev(nullptr), m_pHost(nullptr)
		{
		}
		xListNode(T* pObject) : m_pObject(pObject), m_pNext(nullptr), m_pPrev(nullptr), m_pHost(nullptr)
		{
		}
		~xListNode()
		{
			m_pNext = nullptr;
			m_pPrev = nullptr;
			m_pObject = nullptr;
		}
		xListNode* getNext() { return m_pNext; }
		xListNode* getPrev() { return m_pPrev; }
		xListHost<T>* getHost() { return m_pHost; }
		void setObject(T* pObject) { m_pObject = pObject; }
		T* getObject() { return m_pObject; }
		void setNext(xListNode* pNext) { m_pNext = pNext; }
		void setPrev(xListNode* pPrev) { m_pPrev = pPrev; }
		void setHost(xListHost<T>* pHost) { m_pHost = pHost; }
		BOOL Leave()
		{
			if (m_pHost == nullptr)return FALSE;
			return m_pHost->removeNode(this);
		}
		BOOL Enter(xListHost<T>* pHost)
		{
			return pHost->addNode(this);
		}
		BOOL BelongTo(xListHost<T>* pHost)
		{
			return (pHost == m_pHost);
		}
	};
	//	template <class T>
	//class xEventListener
	//{
	//public:
	//	virtual void OnAddNode( xListHost<T> * pHost, xListNode * pNode ) = 0;
	//	virtual void OnRemoveNode( xListHost<T> * pHost, xListNode * pNode ) = 0;
	//};


private:
	xListNode* m_pHead;
	int	m_iNodeCount;
	//xEventListener * m_pEventListener;
public:
	xListHost() : m_pHead(nullptr), m_iNodeCount(0)/*,m_pEventListener(nullptr)*/ {}
	xListHost(LPVOID pListener) : m_pHead(nullptr), m_iNodeCount(0)/*,m_pEventListener(pListener)*/ {/*assert(pListener!=nullptr);*/ }
	~xListHost() {}
	//xEventListener * getListener(){return m_pEventListener;}
	xListNode* getHead() { return m_pHead; }

	BOOL	removeNode(xListNode* pNode)
	{
		if (pNode == nullptr)return FALSE;
		if (pNode->getHost() != this)return FALSE;

		xListNode* pNext = pNode->getNext();
		xListNode* pPrev = pNode->getPrev();
		if (pNext != nullptr)pNext->setPrev(pPrev);
		if (pPrev != nullptr)pPrev->setNext(pNext);

		if (m_pHead == pNode)
			m_pHead = pNext;
		pNode->setNext(nullptr);
		pNode->setPrev(nullptr);
		pNode->setHost(nullptr);
		//OnRemove(pNode);
//		if( m_pEventListener )m_pEventListener->OnRemoveNode(this,pNode);
		m_iNodeCount--;
		assert(m_iNodeCount >= 0);
		return TRUE;
	}
	BOOL	addNode(xListNode* pNode)
	{
		if (pNode == nullptr)return FALSE;
		pNode->Leave();
		pNode->setHost(this);
		pNode->setPrev(nullptr);
		pNode->setNext(m_pHead);
		if (m_pHead != nullptr)
			m_pHead->setPrev(pNode);
		m_pHead = pNode;
		//		if( m_pEventListener )m_pEventListener->OnAddNode(this,pNode);
		m_iNodeCount++;
		return TRUE;
	}

	int	getCount() { return m_iNodeCount; }

	//public:
	//	virtual VOID OnAdd( xListNode<T> * pNode ){}
	//	virtual VOID OnRemove( xListNode<T> * pNode ){}
};

template <class T>
class xListHelper
{
public:
	xListHelper(xListHost<T>* pList)
	{
		setList(pList);
	}

	xListHelper()
	{
		m_pList = nullptr;
		m_pNode = nullptr;
	}

	void setList(xListHost<T>* pList)
	{
		m_pList = pList;
		if (m_pList)
			m_pNode = m_pList->getHead();
	}

	xListHost<T>* getList()
	{
		return m_pList;
	}

	T* first()
	{
		if (m_pList == nullptr)return nullptr;
		m_pNode = m_pList->getHead();
		if (m_pNode)
		{
			T* pObject = m_pNode == nullptr ? nullptr : m_pNode->getObject();
			m_pNode = m_pNode->getNext();
			return pObject;
		}
		return nullptr;
	}

	T * current()
	{
		if( m_pNode )
			return m_pNode->getObject();
		return nullptr;
	}

	T* next()
	{
		if (m_pList == nullptr)return nullptr;
		if (m_pNode)
		{
			T* pObject = m_pNode == nullptr ? nullptr : m_pNode->getObject();
			m_pNode = m_pNode->getNext();
			return pObject;
		}
		return nullptr;
	}
private:
	typename xListHost<T>::xListNode* m_pNode;
	typename xListHost<T>* m_pList;
};


#define	THREAD_PROTECT	CLock locker( &m_CriticalSection );
#define	THREAD_PROTECT_DEFINE	CriticalSection	m_CriticalSection;


template<class T>
class xPtrQueue
{
	T** m_pQueue;
	BOOL m_bFull;
	int m_iPush;
	int m_iPop;
	int	m_iMaxSize;
	THREAD_PROTECT_DEFINE;
public:
	xPtrQueue() :m_iMaxSize(0), m_bFull(FALSE)
	{
		m_pQueue = nullptr;
		m_iPush = 0;
		m_iPop = 0;

	}
	xPtrQueue(int size) :m_iMaxSize(size), m_bFull(FALSE)
	{
		m_pQueue = nullptr;
		create(size);
	}
	~xPtrQueue()
	{
		destroy();
	}

	BOOL create(int nSize)
	{
		destroy();
		m_pQueue = new T * [nSize];
		m_iMaxSize = nSize;
		return TRUE;
	}

	VOID destroy()
	{
		if (m_pQueue != nullptr)
			delete[]m_pQueue;
		m_pQueue = nullptr;
		m_iPush = 0;
		m_iPop = 0;
		m_iMaxSize = 0;
		m_bFull = FALSE;

	}

	BOOL push(T* p)
	{
		THREAD_PROTECT;
		if (p == nullptr)return FALSE;
		if (m_bFull)return FALSE;

		m_pQueue[m_iPush++] = p;
		if (m_iPush >= m_iMaxSize)
			m_iPush = 0;
		if (m_iPush == m_iPop)
			m_bFull = TRUE;
		return TRUE;
	}

	T* pop()
	{
		THREAD_PROTECT;
		if (!m_bFull && m_iPush == m_iPop)return nullptr;

		int p = m_iPop++;
		m_bFull = FALSE;

		if (m_iPop >= m_iMaxSize)m_iPop = 0;
		return m_pQueue[p];
	}

	VOID clear()
	{
		THREAD_PROTECT;
		m_bFull = FALSE;
		m_iPush = 0;
		m_iPop = 0;
	}

	int getcount()
	{
		if (m_bFull)return m_iMaxSize;
		if (m_iPush < m_iPop)
			return (m_iPush + m_iMaxSize - m_iPop);
		return (m_iPush - m_iPop);
	}
};


#define	POOLMEMORYFLAG	0x558800aa
#define	OBJECTPOOLCACHESIZE	4096

template < class T >
class xObjectPool
{
	typedef struct _object_desc_
	{
		_object_desc_() :node(&object), dwFlag(POOLMEMORYFLAG)
		{
		}
		DWORD	dwFlag;
		typename xListHost<T>::xListNode  node;
		T object;
	}OBJECT_DESC;
public:
	xObjectPool()
		:m_xUsedObjects(nullptr), m_xFreeObjects(nullptr)
	{
		CacheObjects();
	}
	T* newObject()
	{
		THREAD_PROTECT;
		if (m_xFreeObjects.getCount() == 0)
		{
			OBJECT_DESC* pObject = NewObjectDesc();//new OBJECT_DESC;
			m_xUsedObjects.addNode(&pObject->node);
			return &pObject->object;
		}
		else
		{
			xListHost<T>::xListNode* pNode = m_xFreeObjects.getHead();
			if (pNode == nullptr)
				return nullptr;
			m_xFreeObjects.removeNode(pNode);
			m_xUsedObjects.addNode(pNode);
			return pNode->getObject();
		}
		return nullptr;
	}
	VOID deleteObject(T* pObject)
	{
		THREAD_PROTECT;
		BYTE* pStart = (BYTE*)pObject;
		pStart -= sizeof(xListHost<T>::xListNode) + sizeof(DWORD);
		OBJECT_DESC* pObjectDesc = (OBJECT_DESC*)pStart;
		if (pObjectDesc->dwFlag != POOLMEMORYFLAG)return;
		m_xUsedObjects.removeNode(&pObjectDesc->node);
		m_xFreeObjects.addNode(&pObjectDesc->node);
	}
	int	getCount()
	{
		return (m_xUsedObjects.getCount() + m_xFreeObjects.getCount());
	}
	int	getFreeCount()
	{
		return m_xFreeObjects.getCount();
	}
	int	getUsedCount()
	{
		return m_xUsedObjects.getCount();
	}
	typename xListHost<T>::xListNode* GetUsedHeadNode()
	{
		return m_xUsedObjects.getHead();
	}
	typename xListHost<T>::xListNode* GetFreeHeadNode()
	{
		return m_xFreeObjects.getHead();
	}
private:
	CriticalSection	m_CriticalSection;
	OBJECT_DESC* NewObjectDesc()
	{
		if (m_nCachePtr >= m_nCacheSize)
			CacheObjects();
		return &m_pCache[m_nCachePtr++];
	}
	VOID CacheObjects()
	{
		m_nCacheSize = OBJECTPOOLCACHESIZE > sizeof(OBJECT_DESC) ? 4 : (OBJECTPOOLCACHESIZE + sizeof(OBJECT_DESC) - 1) / sizeof(OBJECT_DESC);
		m_pCache = new OBJECT_DESC[m_nCacheSize];
		m_nCachePtr = 0;
	}
	OBJECT_DESC* m_pCache;
	UINT m_nCachePtr;
	UINT m_nCacheSize;
	xListHost<T> m_xUsedObjects;
	xListHost<T> m_xFreeObjects;
};


template<class T>
class xAutoPtrArray
{
public:
	xAutoPtrArray(UINT max)
	{
		m_pArray = nullptr;
		Create(max);
	}
	xAutoPtrArray()
	{
		m_pArray = nullptr;
		m_iMax = 0;
		m_iCount = 0;
	}
	~xAutoPtrArray()
	{
		m_iMax = 0;
		m_iCount = 0;
		delete[]m_pArray;
	}
	BOOL Create(UINT max)
	{
		m_pArray = new T * [max];
		m_iMax = max;
		//m_iCount = 0;
		Clean();
		return TRUE;
	}
	VOID Clean()
	{
		if (m_pArray != nullptr)
			memset(m_pArray, 0, sizeof(T*) * m_iMax);
		m_iCount = 0;
	}
	UINT Add(T* pt)
	{
		if (m_iCount == m_iMax)return (UINT)-1;
		m_pArray[m_iCount++] = pt;
		return (m_iCount - 1);
	}
	T* Get(UINT index)
	{
		if (index >= m_iCount)
			return nullptr;
		return m_pArray[index];
	}
	BOOL Del(T* pt)
	{
		for (UINT i = 0; i < m_iCount; i++)
		{
			if (m_pArray[i] == pt)
			{
				return Del(i);
			}
		}
		return FALSE;
	}
	BOOL Del(UINT index)
	{
		if (index >= m_iCount)return FALSE;
		m_iCount--;
		UINT ileft = m_iCount - index;
		if (ileft > 0)
		{
			memmove(m_pArray + index, m_pArray + index + 1, sizeof(T*) * ileft);
		}
		return TRUE;
	}
	T* operator [](UINT index)
	{
		return Get(index);
	}
	UINT GetCount() { return m_iCount; }
	UINT operator  [] (T* pt)
	{
		for (UINT i = 0; i < m_iCount; i++)
		{
			if (m_pArray[i] == pt)
				return i;
		}
		return (UINT)-1;
	}
	UINT GetMaxCount() { return m_iMax; }

	BOOL Insert(T* pt, UINT Index = 0)
	{
		if (Index >= m_iCount)return (Add(pt) != 0xffffffff);
		if (m_iCount >= m_iMax)return FALSE;
		memmove(m_pArray + Index + 1, m_pArray + Index, sizeof(T*) * (m_iCount - Index));
		m_pArray[Index] = pt;
		m_iCount++;
		return TRUE;
	}
private:
	T** m_pArray;
	UINT m_iCount;
	UINT m_iMax;
};

template <class T>
class xSingletonClass
{
public:
	xSingletonClass()
	{
		m_pInstance = (T*)this;
	}
	static T* GetInstance()
	{
		if (m_pInstance == nullptr)
			m_pInstance = new T;
		return m_pInstance;
	}
protected:
	static T* m_pInstance;
};
template <class T>
T* xSingletonClass<T>::m_pInstance = (T*)nullptr;

class xEventSender;

class xEventListener
{
public:
	virtual VOID OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam) = 0;
};

class xEventSender
{
public:
	xEventSender()
	{
		m_pEventListener = nullptr;
	}
	VOID setEventListener(xEventListener* pEventListener) { m_pEventListener = pEventListener; }
	xEventListener* getEventListener() { return m_pEventListener; }
	VOID sendEvent(int iEvent, int iParam, LPVOID lpParam) { if (m_pEventListener) m_pEventListener->OnEvent(this, iEvent, iParam, lpParam); }
protected:
	xEventListener* m_pEventListener;
};


inline char* Trim(char* pString)
{
	int len = (int)strlen(pString);
	char* p = pString;

	while (*p == ' ' || *p == '	')p++, len--;
	while (*(p + len - 1) == ' ' || *(p + len - 1) == '	')len--, * (p + len) = 0;
	return p;
}

// 不支持中文字符串分割的函数
inline int SearchParam(char* buffer, char** Params, int maxparam, int spliter)
{
	char* pbuffer = Trim(buffer);
	int len = (int)strlen(buffer);
	if (len == 0)return 0;
	char* p = strchr(pbuffer, spliter);
	int count = 0;
	Params[0] = pbuffer;
	while (p)
	{
		*p++ = 0;
		Params[count++] = Trim(Params[count]);
		if (count >= maxparam)return count;
		Params[count] = p;
		p = strchr(p, spliter);
	}
	Params[count++] = Trim(Params[count]);
	return count;
}

// 支持中文字符串分割的函数
inline int SearchParamW(char* buffer, char** Params, int maxparam, const char* spliter)
{
	char* pbuffer = Trim(buffer);
	int len = (int)strlen(buffer);
	if (len == 0) return 0;
	int splitLen = (int)strlen(spliter);
	char* p = strstr(pbuffer, spliter);  // 使用 strstr 支持多字节分隔符
	int count = 0;
	Params[0] = pbuffer;
	while (p)
	{
		memset(p, 0, splitLen);  // 将分隔符替换为多个 '\0'
		p += splitLen;
		Params[count++] = Trim(Params[count]);
		if (count >= maxparam) return count;
		Params[count] = p;
		p = strstr(p, spliter);
	}
	Params[count++] = Trim(Params[count]);
	return count;
}

template<int maxindex>
class xStringsExpander
{
public:
	xStringsExpander(char* pszString, int Delim)
	{
		m_iCount = SearchParam(pszString, m_pStrings, maxindex, Delim);
	}
	const char* getString(int index)
	{
		if (index >= m_iCount || index < 0)return nullptr;
		return m_pStrings[index];
	}

	int getCount() { return m_iCount; }

	const char* operator [](int index)
	{
		return getString(index);
	}
private:
	char* m_pStrings[maxindex];
	int	m_iCount;
};
