#pragma once
#include <vector>

//物品管理
class CItemManager : public xSingletonClass<CItemManager>
{
public:
	CItemManager(void);
	virtual ~CItemManager(void);
	//清空物品数据
	VOID ClearItemData();
	//加载物品数据BaseItem.csv
	VOID Load(const char* pszfile);
	//加载物品限制ItemLimit.txt
	VOID LoadLimit(const char* pszfile);
	//加载物品脚本连接ItemScript.txt
	VOID LoadScriptLink(const char* pszfile);
	//加载豹子等级经验表PetLevel.ini
	VOID LoadPetINI(const char* pszfile);
	// 字符串池管理
	WORD AddStringToPool(const char* pszString);
	// 从池里获取字符串
	const char* GetStringFromPool(WORD wStringId);
	// 清除字符串池
	void ClearStringPool();
	//添加物品类
	BOOL AddItemClassString(const char* pszItemClassDesc);
	// 豹子升级经验, type是豹子类型
	int GetPetLevelInfo(int lv, BYTE type)
	{
		int temp = 0;
		if (type >= PetInfolist.size()) return temp;
		switch (lv)
		{
		case 0:
			temp = PetInfolist[type].lv0;
			break;
		case 1:
			temp = PetInfolist[type].lv1;
			break;
		case 2:
			temp = PetInfolist[type].lv2;
			break;
		case 3:
			temp = PetInfolist[type].lv3;
			break;
		case 4:
			temp = PetInfolist[type].lv4;
			break;
		case 5:
			temp = PetInfolist[type].lv5;
			break;
		case 6:
			temp = PetInfolist[type].lv6;
			break;
		case 7:
			temp = PetInfolist[type].lv7;
			break;
		default:
			temp = 0; // 无效等级返回0
			break;
		}
		return temp;
	}
	//删除物品
	BOOL DeleteItem(DWORD dwMakeIndex);
	//创建物品
	BOOL CreateItem(const char* pszName, DWORD dwId, DWORD dwKey, DWORD dwOwner, WORD wPos, BYTE btFlag, BOOL IsBind = FALSE, BOOL bRandomUpgrade = FALSE);
	//创建临时物品
	BOOL CreateTempItem(const char* pszName, ITEM& item, BOOL bRandomUpgrade = FALSE, int ratefix = 0);
	//鉴别物品
	VOID IdentItem(ITEM& item);
	//	name/stdmode/shape/image/specialpower/ac1/ac2/mac1/mac2/dc1/dc2/mc1/mc2/sc1/sc2/weight/maxdura/price/needtype/needlevel
	//添加物品类
	BOOL AddItemClass(const char* pszName, BYTE stdmode, BYTE shape, WORD wImage, char specialpower,
		BYTE ac1, BYTE ac2, BYTE mac1, BYTE mac2, BYTE dc1, BYTE dc2, BYTE mc1, BYTE mc2, BYTE sc1, BYTE sc2, BYTE weight,
		BYTE maxdura, int price, BYTE needtype, BYTE needlevel) {
		return FALSE;
	}
	//制作模板物品
	BOOL MakeupTemplateItem(const char* pszName, ITEM& item);
	//制作随机属性模板物品
	BOOL MakeupRandomUpgradeItemTemplate(const char* pszName, ITEM& item, int ratefix = 0);
	//获取升级属性值
	VOID GetUpgradeValues(BYTE btStdMode, BYTE btShape, int ratefix, WORD& wDura, WORD& wMaxDura, BYTE* pValues);
	//通过名字获取物品类
	ITEMCLASS* GetItemClassByName(const char* pszName);
	//获取错误信息
	const char* GetErrorMsg() { return m_pErrorMsg; }
	//更新物品位置
	BOOL UpdateItemPos(DWORD dwItemIndex, BYTE btFlag, WORD wPos);
	//更新物品所有者
	BOOL UpdateItemOwner(DWORD dwOwnerId, DWORD dwItemIndex, BYTE	btFlag, WORD wPos);
	//是否是临时物品
	BOOL IsTempItem(DWORD dwItemIndex) { return ((dwItemIndex & 0x80000000) != 0); }
	//添加物品修改标识
	VOID AddItemModifyFlag(ITEM& item, BYTE btFlag);
	//更新物品
	BOOL UpdateItems(DWORD dwOwnerId, BYTE btFlag, DBITEM* pItemArray, int count);
	//更新物品位置
	BOOL UpdateItemPos(BYTE btFlag, BAGITEMPOS* pItemPos, WORD wCount);
	//获取统计数量
	VOID GetMiscCount(DWORD& dwTempCount, DWORD& dwCreateCount, DWORD& deleteCount, DWORD& dwIdentCount) const
	{
		dwTempCount = m_nTempItemCount;
		dwCreateCount = m_nCreateItemCount;
		deleteCount = m_nDeleteTempItemCount;
		dwIdentCount = m_nIdentCount;
	}
	//升级物品
	BOOL UpgradeItem(ITEM& item);
	//减少持久值
	VOID DamageDura(ITEM& item, WORD wDura);
	//检查持久值
	BOOL CheckDura(ITEM& item, WORD wDura);
	//获取物品类
	ITEMCLASS* GetItemClass(ITEM& item);
	//物品是否是限制
	BOOL ItemLimited(ITEM& item, item_limit limit);
	//修改物品
	BOOL ModifyItem(ITEM& item, const char* pszModifyString);
	//升级物品
	BOOL UpgradeItem(ITEM& item, const char* pszUpgradeString);
	//升级宠物物品
	BOOL UpgradePetItem(ITEM& item, const char* pszUpgradeString);
	// 获取ITEMCLASS指针
	inline ITEMCLASS* GetItemClassPtr(ITEM& item)
	{
		if (item.nItemClassPtr == 0) // 如果为空, 自动从新绑定指针
		{
			ITEMCLASS* pItemClass =  GetItemClassByName(item.szFullName);
			SetItemClassPtr(item, pItemClass);
			return pItemClass;
		}
		return reinterpret_cast<ITEMCLASS*>(item.nItemClassPtr);
	}
	// 设置ITEMCLASS指针
	inline VOID SetItemClassPtr(ITEM& item, ITEMCLASS* pClass)
	{
		item.nItemClassPtr = reinterpret_cast<size_t>(pClass);
	}
	// 获取脚本路径
	inline const char* GetItemPage(ITEM& item)
	{
		ITEMCLASS* pClass = GetItemClassPtr(item);
		if (pClass && pClass->wPageId)
			return GetStringFromPool(pClass->wPageId);
		return "";
	}
	// 获取掉落脚本路径
	inline const char* GetDropPage(ITEM& item)
	{
		ITEMCLASS* pClass = GetItemClassPtr(item);
		if (pClass && pClass->wDropPageId)
			return GetStringFromPool(pClass->wDropPageId);
		return "";
	}
	// 获取拾取脚本路径
	inline const char* GetPickupPage(ITEM& item)
	{
		ITEMCLASS* pClass = GetItemClassPtr(item);
		if (pClass && pClass->wPickupPageId)
			return GetStringFromPool(pClass->wPickupPageId);
		return "";
	}
private:
	BOOL AddItemClass(ITEMCLASS* pItemclass);
private:
	xIdAllocorEx<2000000> m_xTempItemIdAllocor;
	xObjectPool<ITEMCLASS> m_xItemClassPool;

	char* m_pErrorMsg;
	
	DWORD m_nIdentCount;
	DWORD m_nTempItemCount;
	DWORD m_nCreateItemCount;
	DWORD m_nDeleteTempItemCount;

	CNameHash m_ItemClassHash;

	std::vector<m_PetInfo> PetInfolist;//宠物升级经验列表

	// 字符串池：使用紧凑存储
	struct StringPoolEntry
	{
		WORD wOffset;     // 在池中的偏移
		WORD wLength;     // 字符串长度
	};
	std::vector<StringPoolEntry> m_vStringIndex;
	std::vector<char> m_vStringData; // 连续存储所有字符串
};