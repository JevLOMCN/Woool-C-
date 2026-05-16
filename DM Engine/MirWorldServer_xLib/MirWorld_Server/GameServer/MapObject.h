#pragma once
#include "vmap.h"
#include "flageventlistener.h"
#include <unordered_map>
#include <list>
#include <vector>

enum e_listnode_index
{
	LNI_AREA,
	LNI_MAP,
	LNI_WORLD,
	LNI_MAX,
};

class CLogicMap;
class CVArea;

class CMapObject : public CFlagEventListener
{
public:
	CMapObject(void);
	virtual ~CMapObject(void);
	xListHost<CMapObject>::xListNode* GetLinkNode(int Index)const
	{
		if (Index >= 0 && Index < LNI_MAX)
			return m_DLinkNodes[Index];
		return nullptr;
	}
	WORD getX()const { return m_wX; }
	WORD getY()const { return m_wY; }
	VOID setX(WORD x) { setXY(x, m_wY); }
	VOID setY(WORD y) { setXY(m_wX, y); }
	VOID setXY(WORD x, WORD y);

	virtual BOOL CanRecvMsg() { return FALSE; }
	virtual VOID OnAroundMsg(CMapObject* pSender, const char* pszCodedMsg, int size) {}
	virtual VOID OnEnterMap(CLogicMap* pMap) { m_pMap = pMap; }
	virtual VOID OnLeaveMap(CLogicMap* pMap) { m_pMap = nullptr; }
	virtual VOID Update() {	}
	virtual VOID Clean();
	virtual BOOL GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr) { return FALSE; }
	virtual BOOL GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr) { return FALSE; }
	//获取对象类型
	virtual e_object_type GetType() { return OBJ_MAX; }
	//获取Appr外观类型
	virtual e_object_ApprType GetApprType() { return APPR_MAX; }
	virtual e_class_type GetClassType() { return CLS_UNKNOWN; }

	UINT GetId() const { return m_Id; }
	VOID SetId(UINT Id) { m_Id = Id; }

	CLogicMap* GetMap() { return m_pMap; }
	virtual VOID OnSetPos(WORD oldx, WORD oldy, WORD newx, WORD newy) {}
	WORD GetInvisibleLevel()const { return m_wInvisibleLevel; }
	virtual WORD GetDeInvisibleLevel() { return 0; }
	DWORD AddRef() { return(++m_dwRefenceCount); }
	DWORD DecRef() { return (m_dwRefenceCount > 0) ? --m_dwRefenceCount : 0; }
	DWORD GetRef()const { return m_dwRefenceCount; }
	VOID SetDelTimer() { m_DeleteTimer.Savetime(); }
	BOOL IsDelTimerTimeOut(DWORD dwTimeOut) { return m_DeleteTimer.IsTimeOut(dwTimeOut); }
	UINT GetMapId()const { return m_Mapid; }
	VOID SetMapId(UINT mapid) { m_Mapid = mapid; }
	VOID SetUpdateKey(DWORD dwKey) { m_dwUpdateKey = dwKey; }
	DWORD GetUpdateKey()const { return m_dwUpdateKey; }

	virtual BOOL InSafeArea();
	BOOL InCityArea();
	BOOL InWarArea();
	virtual VOID OnEnterSafeArea() {}
	virtual VOID OnLeaveSafeArea() {}
	virtual VOID OnEnterCityArea() {}
	virtual VOID OnLeaveCityArea() {}
	virtual VOID OnEnterWarArea() {}
	virtual VOID OnLeaveWarArea() {}
	virtual const char* GetName() { return "MAPOBJECT"; }
	virtual VOID OnMoveTo(UINT ox, UINT oy, UINT nx, UINT ny) {}
protected:
	int EncodeMsg(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize = -1);
	int EncodeMsgRaw(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize = -1);
	CLogicMap* m_pMap;
	CServerTimer m_DeleteTimer;
	DWORD m_dwUpdateKey;
	DWORD m_dwRefenceCount;
	WORD m_wInvisibleLevel;
	WORD m_wX, m_wY;
	UINT m_Mapid;
	UINT m_Id;
	xListHost<CMapObject>::xListNode* m_DLinkNodes[LNI_MAX];
protected: // 消息LRU缓存
	static const int MSG_LPDATA_SIZE = 64; // 只缓存数据部分大小大于128的
	static const int MSG_CACHE_SIZE = 32;    // 消息缓存数量
	struct MessageCacheKey // 消息缓存键结构（用于哈希表）
	{
		DWORD dwFlag;
		WORD wCmd;
		WORD w1;
		WORD w2;
		WORD w3;
		int datasize;
		bool operator==(const MessageCacheKey& other) const
		{
			return dwFlag == other.dwFlag && wCmd == other.wCmd &&
				w1 == other.w1 && w2 == other.w2 && w3 == other.w3 &&
				datasize == other.datasize;
		}
	};
	struct MessageCacheKeyHash // 消息缓存哈希函数
	{
		size_t operator()(const MessageCacheKey& key) const
		{
			size_t h = key.dwFlag;
			h ^= (size_t)key.wCmd << 16;
			h ^= (size_t)key.w1 << 8;
			h ^= (size_t)key.w2;
			h ^= (size_t)key.w3 << 24;
			h ^= (size_t)key.datasize << 8;
			return h;
		}
	};
	struct CachedMessageNode // 消息缓存节点（用于LRU链表）
	{
		DWORD dwFlag;           // 消息标志
		WORD wCmd;              // 命令
		WORD w1;                // 参数1
		WORD w2;                // 参数2
		WORD w3;                // 参数3
		CHAR lpdata[MSG_LPDATA_SIZE];// 数据
		int datasize;           // 数据大小
		std::vector<CHAR> buffer; // 消息缓冲区（动态大小）
		int encodedSize;        // 编码后大小
	};
	class MessageCache // 消息缓存管理类（哈希表+LRU链表）
	{
	private:
		typedef std::list<CachedMessageNode> LRUNodeList;
		typedef std::unordered_map<MessageCacheKey, LRUNodeList::iterator, MessageCacheKeyHash> CacheMap;
		LRUNodeList m_lruList;              // LRU链表（头部最新, 尾部最旧）
		CacheMap m_cacheMap;                // 哈希表
		size_t m_maxSize;                   // 最大缓存数量
	public:
		MessageCache() : m_maxSize(MSG_CACHE_SIZE) {}
		// 查找缓存, 返回nullptr表示未命中或超时
		const CachedMessageNode* Find(const MessageCacheKey& key, LPVOID lpdata)
		{
			auto it = m_cacheMap.find(key);
			if (it == m_cacheMap.end()) return nullptr;
			// 检查数据部分是否匹配（如果有数据）
			if (lpdata != nullptr && key.datasize > 0 && it->second->datasize == key.datasize)
			{
				if (memcmp(it->second->lpdata, lpdata, key.datasize) != 0)
					return nullptr;
			}
			// 命中：移到链表头部（更新LRU）
			m_lruList.splice(m_lruList.begin(), m_lruList, it->second);
			return &(*it->second);
		}
		// 添加或更新缓存
		void Put(const MessageCacheKey& key, LPVOID lpdata, const CHAR* pBuffer, int size)
		{
			// 如果已存在, 更新并移到头部
			auto it = m_cacheMap.find(key);
			if (it != m_cacheMap.end())
			{
				it->second->dwFlag = key.dwFlag;
				it->second->wCmd = key.wCmd;
				it->second->w1 = key.w1;
				it->second->w2 = key.w2;
				it->second->w3 = key.w3;
				if (lpdata && key.datasize > 0 && key.datasize <= MSG_LPDATA_SIZE)
					memcpy(it->second->lpdata, lpdata, key.datasize);
				it->second->datasize = key.datasize;
				it->second->buffer.assign(pBuffer, pBuffer + size);
				it->second->encodedSize = size;
				m_lruList.splice(m_lruList.begin(), m_lruList, it->second);
				return;
			}
			// 检查容量, 移除最久未使用的项
			if (m_lruList.size() >= m_maxSize)
			{
				const CachedMessageNode& last = m_lruList.back();
				MessageCacheKey lastKey{ last.dwFlag, last.wCmd, last.w1, last.w2, last.w3, last.datasize };
				m_cacheMap.erase(lastKey);
				m_lruList.pop_back();
			}
			// 插入新节点到链表头部
			CachedMessageNode node{};
			node.dwFlag = key.dwFlag;
			node.wCmd = key.wCmd;
			node.w1 = key.w1;
			node.w2 = key.w2;
			node.w3 = key.w3;
			if (lpdata && key.datasize > 0 && key.datasize <= MSG_LPDATA_SIZE)
				memcpy(node.lpdata, lpdata, key.datasize);
			node.datasize = key.datasize;
			node.buffer.assign(pBuffer, pBuffer + size);
			node.encodedSize = size;
			m_lruList.push_front(node);

			m_cacheMap[key] = m_lruList.begin();
		}
	};
	// 智能消息编码：使用LRU缓存策略减少重复编码
	VOID SmartEncodeMessage(CHAR* pBuffer, int& size, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize = -1);
	// 消息缓存（使用thread_local, 每个线程独立）
	thread_local static MessageCache s_msgCache;
};
