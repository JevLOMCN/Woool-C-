#include "xinc.h"

class xPacket
{
public:
	xPacket()
	{
		m_pBuf = nullptr;
		m_iSize = 0;
		m_iMaxSize = 0;
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
	}

	xPacket(int nSize)
	{
		m_pBuf = nullptr;
		m_iSize = 0;
		m_iMaxSize = 0;
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
		create(nSize);
	}

	xPacket(char* pbuf, int nSize)
	{
		m_pBuf = nullptr;
		m_iSize = 0;
		m_iMaxSize = 0;
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
		create(pbuf, nSize);
	}

	virtual ~xPacket(void)
	{
		destroy();
	}

	BOOL create(char* pbuf, int nSize)
	{
		destroy();
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
		m_pBuf = pbuf;
		m_iMaxSize = nSize;
		return TRUE;
	}

	BOOL create(int nSize)
	{
		destroy();
		if (nSize > 0)
		{
			m_bBuildInBuffer = TRUE;
			// 使用16字节对齐的内存分配, 提升缓存性能
			m_pBuf = (char*)_aligned_malloc(nSize, 16);
			if (m_pBuf) {
				m_iMaxSize = nSize;
				m_bAligned = TRUE;
			} else {
				// 回退到标准分配
				m_pBuf = new char[nSize];
				m_iMaxSize = nSize;
				m_bAligned = FALSE;
			}
		}
		return TRUE;
	}

	BOOL notcreated()const
	{
		return (m_pBuf == nullptr);
	}

	VOID destroy()
	{
		if (m_bBuildInBuffer && m_pBuf) {
			// 根据分配方式释放内存
			if (m_bAligned) {
				_aligned_free(m_pBuf);
			} else {
				delete[] m_pBuf;
			}
		}
		m_pBuf = nullptr;
		m_iMaxSize = 0;
		m_iSize = 0;
		m_bBuildInBuffer = FALSE;
		m_bAligned = FALSE;
	}

	BOOL push(LPVOID lpData, int iDatasize)
	{
		if (m_iMaxSize - m_iSize < iDatasize)return FALSE;
		// 优化的内存复制
		if (iDatasize >= 64 && ((uintptr_t)m_pBuf % 8) == 0 && ((uintptr_t)lpData % 8) == 0) {
			// 8字节对齐的大块数据, 使用快速复制
			uint64_t* dst = (uint64_t*)(m_pBuf + m_iSize);
			const uint64_t* src = (uint64_t*)lpData;
			int count = iDatasize / 8;
			for (int i = 0; i < count; ++i) {
				dst[i] = src[i];
			}
			// 处理剩余字节
			int remaining = iDatasize % 8;
			if (remaining > 0) {
				memcpy((char*)dst + count * 8, (char*)src + count * 8, remaining);
			}
		} else {
			// 小块数据或未对齐, 使用标准memcpy
			memcpy(m_pBuf + m_iSize, lpData, iDatasize);
		}
		
		m_iSize += iDatasize;
		return TRUE;
	}

	BOOL push(const char* pszString)
	{
		size_t len = strlen(pszString);
		return push((LPVOID)pszString, (int)len);
	}

	BOOL pop(LPVOID lpData, int iDatasize)
	{
		if (m_iSize < iDatasize)return FALSE;
		memcpy(lpData, m_pBuf, iDatasize);
		return free(iDatasize);
	}

	BOOL push(int nZero)
	{
		if (m_iMaxSize - m_iSize < nZero) return FALSE;
		memset(m_pBuf + m_iSize, 0, nZero);
		m_iSize += nZero;
		return TRUE;
	}

	BOOL peek(LPVOID lpData, int iDatasize)const
	{
		if (m_iSize < iDatasize)return FALSE;
		memcpy(lpData, m_pBuf, iDatasize);
		return TRUE;
	}

	BOOL free(int iDatasize)
	{
		if (m_iSize < iDatasize)return FALSE;
		m_iSize -= iDatasize;
		if (m_iSize != 0) {
			// 对于小块数据使用 memcpy, 大块数据优化移动策略
			if (iDatasize < 256) {
				// 小块数据, 直接移动
				memmove(m_pBuf, m_pBuf + iDatasize, m_iSize);
			} else {
				// 大块数据, 使用更高效的内存移动
				char* temp = m_pBuf;
				if (iDatasize < m_iSize) {
					// 检查是否可以用 8 字节对齐的快速移动
					if (((uintptr_t)temp & 7) == 0 && ((uintptr_t)(m_pBuf + iDatasize) & 7) == 0) {
						// 8 字节对齐, 使用快速复制
						uint64_t* src = (uint64_t*)(m_pBuf + iDatasize);
						uint64_t* dst = (uint64_t*)m_pBuf;
						int count = m_iSize / 8;
						for (int i = 0; i < count; ++i) {
							dst[i] = src[i];
						}
						// 处理剩余字节
						int remaining = m_iSize % 8;
						if (remaining > 0) {
							memcpy((char*)dst + count * 8, (char*)src + count * 8, remaining);
						}
					} else {
						// 回退到标准 memmove
						memmove(m_pBuf, m_pBuf + iDatasize, m_iSize);
					}
				}
			}
		}
		return TRUE;
	}

	VOID clear()
	{
		m_iSize = 0;
	}

	int	getsize()const { return m_iSize; }
	const char* getbuf()const { return m_pBuf; }
	const char* getfreebuf()const { return (m_pBuf + m_iSize); }
	int	getmaxsize()const { return m_iMaxSize; }
	int	getfreesize()const { return m_iMaxSize - m_iSize; }
	void setsize(int nSize) { m_iSize = nSize; }
	BOOL addsize(int nSize)
	{
		if (nSize > getfreesize())return FALSE;
		m_iSize += nSize;
		return TRUE;
	}
	// 禁用拷贝构造函数和拷贝赋值运算符
	xPacket(const xPacket&) = delete;
	xPacket& operator=(const xPacket&) = delete;
private:
	char* m_pBuf;
	int	m_iSize;
	int	m_iMaxSize;
	BOOL m_bBuildInBuffer;
	BOOL m_bAligned;  // 标记是否使用对齐内存分配
};