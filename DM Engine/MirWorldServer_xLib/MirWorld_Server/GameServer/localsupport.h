#pragma	once

#define	INDEXEDTABLE_SEGMENT_CACHE	2048
template<int MAXCOUNT>
class xIdAllocorEx
{
public:
	xIdAllocorEx()
	{
		memset(m_IdCache, 0, sizeof(m_IdCache));
		m_nIdPtr = 0;
		m_nCacheCount = 0;
	}
	UINT allocid()
	{
		if (m_nCacheCount > 0)
			return m_IdCache[--m_nCacheCount];
		return (++m_nIdPtr);
	}
	VOID freeid(UINT id)
	{
		if (id == m_nIdPtr)
			m_nIdPtr--;
		else if (m_nCacheCount < MAXCOUNT)
			m_IdCache[m_nCacheCount++] = id;
	}
	BOOL isused(UINT id)
	{
		return TRUE;
	}
	//	UINT getcount(){ return m_iCount;}
private:
	UINT m_IdCache[MAXCOUNT];
	UINT m_nCacheCount;
	UINT m_nIdPtr;
};
template<int MAXCOUNT>
class xIdAllocor
{
public:
	xIdAllocor()
	{
		for (UINT i = 0; i < MAXCOUNT; i++)
		{
			m_NextFree[i] = i + 1;
		}
		m_iFree = 0;
		m_iCount = 0;
	}
	UINT allocid()
	{
		if (m_iFree >= MAXCOUNT)return 0;
		UINT id = m_iFree + 1;
		m_iFree = m_NextFree[m_iFree];
		m_NextFree[id - 1] = 0xffffffff;
		m_iCount++;
		return id;
	}
	VOID freeid(UINT id)
	{
		if (id == 0 || id > MAXCOUNT)return;
		id--;
		if (m_NextFree[id] != 0xffffffff)return;
		m_NextFree[id] = m_iFree;
		m_iFree = id;
		m_iCount--;
	}
	BOOL isused(UINT id)
	{
		return (id > 0 && id <= MAXCOUNT && m_NextFree[id - 1] == 0xffffffff);
	}
	UINT getcount() { return m_iCount; }
	BOOL isfull() { return (m_iCount >= MAXCOUNT); }
private:
	UINT m_NextFree[MAXCOUNT + 1];
	UINT m_iFree;
	UINT m_iCount;
};
template<class T>
class xIndexedTable
{
	struct table_array
	{
		UINT nStartId;
		xIdAllocor<INDEXEDTABLE_SEGMENT_CACHE>	idallocor;
		T* array[INDEXEDTABLE_SEGMENT_CACHE];
		table_array* pNext;
	};
public:
	xIndexedTable()
	{
		m_pArray = nullptr;
		m_iCount = 0;
	}
	UINT add(T* pt)
	{
		table_array* pFreeArray = getfreearray();
		if (pFreeArray == nullptr)return 0;
		UINT id = pFreeArray->idallocor.allocid();
		if (id == 0)return 0;
		pFreeArray->array[id - 1] = pt;
		m_iCount++;
		return pFreeArray->nStartId + id;
	}
	T* get(UINT id)
	{
		table_array* p = getarray(id);
		if (p == nullptr)return nullptr;
		id -= p->nStartId;
		if (!p->idallocor.isused(id))return nullptr;
		return p->array[id - 1];
	}
	VOID del(UINT id)
	{
		table_array* p = getarray(id);
		if (p == nullptr)return;
		id -= p->nStartId;
		if (!p->idallocor.isused(id))return;
		p->array[id - 1] = nullptr;
		p->idallocor.freeid(id);
		m_iCount--;
	}
private:
	table_array* getarray(UINT id)
	{
		table_array* p = m_pArray;
		while (p)
		{
			if (id > p->nStartId)
			{
				return p;
			}
			p = p->pNext;
		}
		return nullptr;
	}
	table_array* getfreearray()
	{
		table_array* p = m_pArray;
		while (p)
		{
			if (!p->idallocor.isfull())
			{
				//pfree = p;
				return p;
			}
			p = p->pNext;
		}
		p = new table_array;
		if (p == nullptr)return p;
		if (m_pArray != nullptr)
			p->nStartId = m_pArray->nStartId + INDEXEDTABLE_SEGMENT_CACHE;
		else
			p->nStartId = 0;
		p->pNext = m_pArray;
		m_pArray = p;
		return p;
	}
	table_array* m_pArray;
	int	m_iCount;
};

template <int maxcount>
class xSortedIntegerArray
{
public:
	BOOL add(UINT integer)
	{
	}
	BOOL has(UINT integer)
	{
	}
	VOID remove(UINT integer)
	{
	}
private:
	UINT m_iArray[maxcount];
	int	m_iCount;
};

static char* gettoken(char* pString)
{
	char* p = pString;
	while (*p && (*p == ' ' || *p == '	'))p++;
	if (*p == 0)
		return nullptr;
	BOOL	bString = FALSE;
	while (*p && ((*p != ' ' && *p != '	') || bString))
	{
		if (*p == '\"')
		{
			bString = !bString;
		}
		p++;
	}
	if (*p != 0)
		*p++ = 0;
	return p;
}

inline int getintegerinside(char* buffer, int begin, int end)
{
	char* p = buffer;

	char* pret = nullptr;
	int iret = 0;
	while (*p)
	{
		if (*p == begin)
			pret = p + 1;
		if (pret != nullptr && *p == end)
		{
			*p = 0;
			iret = StringToInteger(pret);
			*p = end;
			return iret;
		}
		p++;
	}
	return 0;
}

inline char* getstringinside(char* buffer, int begin, int end)
{
	char* p = buffer;
	char* pret = nullptr;
	while (*p)
	{
		if (*p == begin)
			pret = p + 1;
		if (pret != nullptr && *p == end)
		{
			*p = 0;
			return pret;
		}
		p++;
	}
	return nullptr;
}

inline char* gettoedge(char* buffer, int begin, int end)
{
	char* p = buffer;
	int level = 0;
	while (*p)
	{
		if (*p == begin)
		{
			level++;
		}
		else if (*p == end)
		{
			level--;
			if (level == 0)
				return (p + 1);
		}
	}
	return nullptr;
}

inline int StringToStruct(const char* pszString, LPVOID lpStruct)
{
	char* Params[200];
	char* subParams[100];
	int nSubParam = 0;
	char	szBuffer[2048];
	char* pBinbuffer = (char*)lpStruct;
	int binPtr = 0;
	o_strncpy(szBuffer, pszString, 2047);
	int nParam = SearchParam(szBuffer, Params, 200, '|');
	//	d[2]:{ 0x91, 0x88}|s[16]:abcdefg|s[100]:gamesoft|s[60]
	bool bArray = false;
	int	arraysize = 0;
	//	d * 2 : { 0x91, 0x88 }

	for (int i = 0; i < nParam; i++)
	{
		bArray = false;
		arraysize = 1;
		char c = *Params[i]++;
		c = tolower(c);

		if (*Params[i] == '[')
		{
			//	处理数组
			bArray = true;
			arraysize = getintegerinside(Params[i], '[', ']');
			if (arraysize == 0)return 0;
			Params[i] = gettoedge(Params[i], '[', ']');
			if (Params[i] == nullptr)return 0;
		}
		if (*Params[i] == ':')
		{
			(*Params[i])++;
		}
		else
			return 0;
		//	对于字符串, 不离参数.
		if (c == 's')
		{
			strcpy(pBinbuffer + binPtr, Params[i]);
			if (bArray)
				binPtr += arraysize;
			else
				binPtr += (int)strlen(Params[i]);
			continue;
		}

		int blocksize = 0;

		switch (c)
		{
		case	'w':
			blocksize = 2;
			break;
		case	'd':
			blocksize = 4;
			break;
		case	'b':
			blocksize = 1;
			break;
		default:
			blocksize = 4;
		}
		int value;

		if (bArray)
		{
			//	开始分开数组参数
			Params[i] = getstringinside(Params[i], '{', '}');
			if (Params[i] == nullptr)
				return 0;
			nSubParam = SearchParam(Params[i], subParams, 100, ',');
			if (nSubParam != arraysize)
			{
				if (nSubParam == 0)return 0;
				arraysize = nSubParam;
			}
			for (int block = 0; block < arraysize; block++)
			{
				value = StringToInteger(subParams[block]);
				memcpy(pBinbuffer + binPtr, &value, blocksize);
				binPtr += blocksize;
			}
		}
		else
		{
			value = StringToInteger(Params[i]);
			memcpy(pBinbuffer + binPtr, &value, blocksize);
			binPtr += blocksize;
		}
	}
	return binPtr;
}

static int	GetMsgFromString(const char* pszString, char* pMsgBuffer)
{
	MIRMSG header{};
	char* pMsg = pMsgBuffer;
	char* Params[200];
	char szBuffer[2048];
	char szBinBuffer[1024] = { 0 };
	int	binPtr = 0;
	o_strncpy(szBuffer, pszString, 2047);

	int nParam = SearchParam(szBuffer, Params, 200, '|');
	if (nParam < 5)return 0;

	header.dwFlag = (DWORD)StringToInteger(Params[0]);
	header.wCmd = (WORD)StringToInteger(Params[1]);
	header.wParam[0] = (WORD)StringToInteger(Params[2]);
	header.wParam[1] = (WORD)StringToInteger(Params[3]);
	header.wParam[2] = (WORD)StringToInteger(Params[4]);
	*pMsg++ = '#';
	pMsg += _CodeGameCode((BYTE*)&header, MSGHEADERSIZE, (BYTE*)pMsg);
	char	c;
	for (int i = 5; i < nParam; i++)
	{
		if (*(Params[i] + 1) == ':')
		{
			c = *Params[i];
			Params[i] += 2;
			switch (c)
			{
			case	'w':
			case	'W':
				*(WORD*)(szBinBuffer + binPtr) = (WORD)StringToInteger(Params[i]);
				binPtr += 2;
				break;
			case	'D':
			case	'd':
				*(DWORD*)(szBinBuffer + binPtr) = (DWORD)StringToInteger(Params[i]);
				binPtr += 4;
				break;
			case	'B':
			case	'b':
				*(BYTE*)(szBinBuffer + binPtr) = (BYTE)StringToInteger(Params[i]);
				binPtr += 1;
				break;
			case	's':
			case	'S':
				strcpy(szBinBuffer + binPtr, Params[i]);
				binPtr += (int)strlen(Params[i]);
				break;
			default:
				break;
			}
			continue;
		}
		strcpy(szBinBuffer + binPtr, Params[i]);
		binPtr += (int)strlen(Params[i]);
	}
	if (binPtr > 0)
	{
		*(szBinBuffer + binPtr) = 0;
		pMsg += _CodeGameCode((BYTE*)szBinBuffer, binPtr, (BYTE*)pMsg);
	}
	*pMsg++ = '!';
	return (int)(pMsg - pMsgBuffer);
}

static UINT g_nnUnitIndex = 0;
template <class PT, UINT nIncreaseMeter>
class xDynamicPtrArray
{
public:
	//typedef T PT;
	xDynamicPtrArray(void)
	{
		memset(&m_empty, 0, sizeof(PT));
		m_pArray = nullptr;
		m_nCurrentArraySize = 0;
		m_nCurrentItemCount = 0;
	}
	~xDynamicPtrArray(void)
	{
		if (m_pArray)
			delete[]m_pArray;
	}
	BOOL add(PT pt, UINT& nIndex = g_nnUnitIndex)
	{
		UINT nLeft = m_nCurrentArraySize - m_nCurrentItemCount;
		if (nLeft == 0)
		{
			if (!resizeArray(m_nCurrentArraySize + nIncreaseMeter))
				return FALSE;
		}
		nIndex = m_nCurrentItemCount;
		m_pArray[m - nCurrentItemCount++] = pt;
		return TRUE;
	}
	BOOL insert(UINT nIndex, PT pt)
	{
		UINT nLeft = m_nCurrentArraySize - m_nCurrentItemCount;
		if (nLeft == 0)
		{
			if (!resizeArray(m_nCurrentArraySize + nIncreaseMeter))
				return FALSE;
		}
		if (nIndex >= m_nCurrentItemCount)
			nIndex = m_nCurrentItemCount;
		else
		{
			memmove(m_pArray + nIndex + 1, m_pArray + nIndex, sizeof(PT) * (m_nCurrentItemCount - nIndex));
		}
		m_nCurrentItemCount++;
		m_pArray[nIndex] = pt;
		return TRUE;
	}
	VOID remove(UINT nIndex)
	{
		if (!verifyIndex(nIndex))return;
		m_nCurrentItemCount--;
		if (nIndex < m_nCurrentItemCount)
		{
			memmove(m_pArray + nIndex, m_pArray + nIndex + 1, sizeof(PT) * (m_nCurrentItemCount - nIndex));
		}
	}
	PT& operator[](UINT nIndex)
	{
		if (!verifyIndex(nIndex))return m_empty;
		return m_pArray[nIndex];
	}
	UINT getCount() { return m_nCurrentItemCount; }
protected:
	BOOL verifyIndex(UINT nIndex)
	{
		if (nIndex >= m_nCurrentItemCount)
			return FALSE;
		return TRUE;
	}
	BOOL resizeArray(UINT nNewSize)
	{
		PT* array = new PT[nNewSize];
		if (m_pArray)
		{
			memcpy(array, m_pArray, sizeof(PT) * m_nCurrentItemCount);
			delete[]m_pArray;
		}
		m_pArray = array;
		m_nCurrentArraySize = nNewSize;
		return TRUE;
	}
	PT* m_pArray;
	UINT m_nCurrentArraySize;
	UINT m_nCurrentItemCount;
	PT m_empty;
};

template <class T>
class xKeyPtrList
{
	typedef struct tagKeyObj
	{
		tagKeyObj()
		{
			FILLSELF(0);
		}
		DWORD dwKey;
		T* pObject;
	}KeyObj;
public:
	xKeyPtrList()
	{
	}
	~xKeyPtrList()
	{
	}
	BOOL addObject(DWORD dwKey, T* pObject)
	{
		UINT index = getFitableIndex(dwKey);
		KeyObj obj;
		obj.pObject = pObject;
		obj.dwKey = dwKey;
		return m_xObjectArray.insert(index, obj);
	}
	T* getObject(DWORD dwKey)
	{
		UINT index = getFitableIndex(dwKey);
		if (m_xObjectArray[index].dwKey == dwKey)
			return m_xObjectArray[index].pObject;
		return nullptr;
	}
	DWORD getObjectKey(T* pObject)
	{
		if (getObjectIndex(pObject) == (UINT)-1)return 0;
		return m_xObjectArray[i].dwKey;
	}
	BOOL removeObject(DWORD dwKey)
	{
		UINT index = getFitableIndex(dwKey);
		if (m_xObjectArray[i].dwKey != dwKey)return FALSE;
		m_xObjectArray.remove(index);
		return TRUE;
	}
	BOOL removeObject(T* pObject)
	{
		UINT index = getObjectIndex(pObject);
		if (index == (UINT)-1)return FALSE;
		m_xObjectArray.remove(index);
		return TURE;
	}
protected:
	UINT getObjectIndex(T* pObject)
	{
		UINT i = 0;
		for (i = 0; i < m_xObjectArray.getCount(); i++)
		{
			if (pObject == m_xObjectArray[i].pObject)
				return i;
		}
		return -1;
	}
	UINT getFitableIndex(DWORD dwKey)
	{
		UINT s = 0, e = m_xObjectArray.getCount(), m = 0;
		if (e == 0)return 0;
		while (s < (e + 1))
		{
			m = (s + e) / 2;
			if (dwKey > m_xObjectArray[m].dwKey)
				s = m;
			else
				e = m;
		}
		if (s == m)return e;
		return s;
	}
	xDynamicPtrArray<KeyObj, 512 > m_xObjectArray;
};

// 获取tt的方向
inline int GetFlyDirection(int sx, int sy, int ttx, int tty)
{
	int dx = ttx - sx;
	int dy = tty - sy;

	if (dx == 0) return (dy > 0) ? 4 : 0;
	if (dy == 0) return (dx > 0) ? 2 : 6;

	int absDx = abs(dx);
	int absDy = abs(dy);

	if (absDx > absDy * 3) return (dx > 0) ? 2 : 6;
	if (absDy > absDx * 3) return (dy > 0) ? 4 : 0;

	if (dx > 0) return (dy > 0) ? 3 : 1;
	else return (dy > 0) ? 5 : 7;
}

// 计算字符串数量
inline int countStringChar(const char* pstring, int ichar)
{
	char* p = (char*)pstring;
	int c = 0;
	while (*p)
	{
		if (*p == ichar)c++;
		p++;
	}
	return c;
}

inline void replaceOutPair(char* pszString, int PairL, int PairR, int ReplaceTo)
{
	char* p = pszString;
	int	PairLevel = 0;
	int tLevel = 0;
	BOOL	bErase = FALSE;
	while (*p)
	{
		if (*p == ' ' || *p == '\t')
		{
			if (tLevel == 0)
				tLevel = 1;
		}
		else if (*p == PairL)
		{
			PairLevel++;
			if (PairLevel == 1 && tLevel != 2)
			{
				*p = ReplaceTo;
				bErase = TRUE;
			}
		}
		else if (*p == PairR)
		{
			if (PairLevel == 1 && bErase)
				*p = ReplaceTo;
			PairLevel--;
		}
		else
		{
			if (tLevel == 1)
				tLevel = 2;
		}
		p++;
	}
}

inline static BOOL InRect(int x, int y, RECT& rc)
{
	if (x < rc.left || x > rc.right)return FALSE;
	if (y < rc.top || y > rc.bottom)return FALSE;
	return TRUE;
}

// 过滤物品显示名字的末尾数字
inline int FilterItemShowName(char* sName)
{
	if (!sName) return 0;
	int len = 0;
	while (len < 256 && sName[len] != '\0') // 限制最大长度
		len++;
	if (len == 0) return 0;
	int pos = len - 1;
	while (pos >= 0) // 从后往前找第一个非数字字符
	{
		if (sName[pos] < '0' || sName[pos] > '9')
			break;
		if (pos == 0)
		{
			sName[0] = '\0';
			return 0;
		}
		pos--;
	}
	// 设置结束符
	if (pos >= 0)
	{
		sName[pos + 1] = '\0';
		return pos + 1;
	}
	return 0;
}

// 字符转字节集
inline void CharToBytes(const char* str, BYTE* outBytes, int& outCount)
{
	if (str == nullptr || outBytes == nullptr)
	{
		outCount = 0;
		return;
	}
	int len = (int)strlen(str);
	outCount = min(len, 256); // 限制最大256字节
	for (int i = 0; i < outCount; i++)
	{
		outBytes[i] = (BYTE)(str[i] & 0xff);
	}
}

// 字符转字节集并打印的函数
// 示例:
// BYTE bytes[256];
// int count;
// CharToBytesAndPrint("ABC", bytes, count);
// 输出: [65,66,67,]
inline void CharToBytesAndPrint(const char* str, BYTE* outBytes, int& outCount)
{
	CharToBytes(str, outBytes, outCount);
	LG1("[");
	for (int i = 0; i < outCount; i++)
	{
		LG1("%d,", outBytes[i]);
	}
	LG1("]\n\n");
}

// 获取金币外形
inline WORD GetGoldImageIndex(DWORD dwCount)
{
	//使用查找表+二分查找
	static const DWORD thresholds[] = { 100, 300, 500, 1000, UINT_MAX };
	static const WORD images[] = { 0xe1, 0xe2, 0xe3, 0xe4, 0xe5 };
	int idx = 0;
	while (idx < 4 && dwCount > thresholds[idx + 1])
		idx++;
	return images[idx];
}

// 检查字符是否为int数字
inline bool IsIntegerNumber(const char* str) 
{
	if (str == nullptr || *str == '\0') return false;
	int i = 0;
	if (str[i] == '-' || str[i] == '+')
	{
		i++;
		if (str[i] == '\0') return false;
	}
	while (str[i] != '\0') 
	{
		unsigned char c = str[i];
		if (!isdigit(c)) return false;
		i++;
	}
	return true;
}

// 比较两个时间戳, 是否已经到第二天
inline int IsNextDay(time_t timestamp1, time_t timestamp2)
{
	if (timestamp1 == 0 || timestamp2 == 0)
		return 1;
	time_t days1 = timestamp1 / ONE_DAY_SECONDS;
	time_t days2 = timestamp2 / ONE_DAY_SECONDS;
	return static_cast<int>(abs(days2 - days1));
}

// 正方形坐标
struct SquareArea
{
	int centerX, centerY = 0;      // 中心坐标
	int sideLength = 0;            // 边长
	std::vector<std::pair<int, int>> outerSquare;  // 外圈正方形坐标
	std::vector<std::pair<int, int>> innerSquare;  // 内圈正方形坐标
};

//计算正方形坐标 - 外圈周长 + 内部小正方形
inline SquareArea CalculateSquareArea(int centerX, int centerY)
{
	SquareArea area;
	area.centerX = centerX;
	area.centerY = centerY;
	area.sideLength = 5;  // 5x5外圈

	// 外圈周长：5x5正方形的边界, 共16个点
	area.outerSquare.resize(16);
	int idx = 0;
	// 上边（从左到右, 5个点）
	for (int x = centerX - 2; x <= centerX + 2; x++)
		area.outerSquare[idx++] = { x, centerY - 2 };
	// 右边（从上到下, 排除右上角, 4个点）
	for (int y = centerY - 1; y <= centerY + 2; y++)
		area.outerSquare[idx++] = { centerX + 2, y };
	// 下边（从右到左, 排除右下角, 4个点）
	for (int x = centerX + 1; x >= centerX - 2; x--)
		area.outerSquare[idx++] = { x, centerY + 2 };
	// 左边（从下到上, 排除左下角和左上角, 3个点）
	for (int y = centerY + 1; y >= centerY - 1; y--)
		area.outerSquare[idx++] = { centerX - 2, y };

	// 内部小正方形：3x3, 共9个点
	area.innerSquare.resize(9);
	idx = 0;
	for (int y = centerY - 1; y <= centerY + 1; y++)
	{
		for (int x = centerX - 1; x <= centerX + 1; x++)
		{
			area.innerSquare[idx++] = { x, y };
		}
	}

	return area;
}