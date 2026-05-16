#pragma once
class CHumanPlayer;
// 穿戴装备
class CEquipment
{
public:
	// 构造函数, 需要传入玩家对象指针
	CEquipment(CHumanPlayer* pPlayer);
	// 析构函数
	virtual ~CEquipment(void);
	// 清空所有装备数据
	VOID Clean();
public:
	// 装备物品到指定位置
	// pos: 装备位置(0-13)
	// inItem: 要装备的物品
	// outItem: 返回被替换下来的物品
	// bForce: 是否强制装备(忽略检查)
	// bNoticePlayer: 是否通知玩家
	BOOL EquipItem(int pos, ITEM& inItem, ITEM& outItem, BOOL bForce = FALSE, BOOL bNoticePlayer = TRUE);
	// 卸下指定位置的装备
	BOOL UnEquipItem(int pos, ITEM& outItem);
	// 检查物品是否可以装备到指定位置
	BOOL CheckFitable(int pos, ITEM& inItem);
	// 检查是否装备了指定物品
	BOOL CheckItemInfo(int pos, BYTE stdMode, BYTE btShape);
public:
	// 重载[]运算符, 方便获取指定位置的装备
	ITEM& operator[](int pos) { if (!VerifyPos(pos))return m_EmptyEquipment; return m_Equipments[pos]; }
public:
	// 获取指定位置的装备指针
	ITEM* GetEquipment(int pos) { if (!VerifyPos(pos) || m_Equipments[pos].dwMakeIndex == 0)return nullptr; return &m_Equipments[pos]; }
	// 计算装备总重量(排除武器和指定位置)
	int	CalcEquipmentsWeight(int expos)const
	{
		int retweight = 0;
		for (int i = 0; i < _U_MAX; i++)
		{
			if (i == _U_WEAPON || i == expos || m_Equipments[i].dwMakeIndex == 0)continue;
			retweight += m_Equipments[i].baseitem.btWeight;
		}
		return retweight;
	}
	// 获取错误信息
	const char* GetErrorMsg()const { return m_strErrorMsg.c_str(); }
	// 设置错误消息
	void SetErrorMsg(const char* pErrorMsg){ m_strErrorMsg = pErrorMsg ? pErrorMsg : ""; }
	// 获取指定属性值
	int	GetProp(PROP_INDEX index) { if (index < 0 || index >= PI_PROP_COUNT)return 0; return m_PropMap[index]; }
	// 重置属性缓存
	VOID ResetPropCache();
	// 获取标准模式对应的属性含义
	BOOL GetStdModeMeaning(BYTE stdMode, int& dc1p, int& dc2p, int& mc1p, int& mc2p, int& sc1p, int& sc2p, int& ac1p, int& ac2p, int& mac1p, int& mac2p);
private:
	// 验证位置是否有效
	BOOL VerifyPos(int pos) { if (pos < 0 || pos >= _U_MAX) return FALSE; return TRUE; }
	// 检查物品类型是否适合装备到指定位置
	BOOL CheckStdModeFitPos(BYTE stdMode, int pos);
	// 检查装备需求信息(等级等)
	BOOL CheckNeedInfo(BYTE needtype, BYTE needlevel);
	// 添加物品属性到缓存
	VOID AddPropCache(ITEM& item);
	// 从缓存中移除物品属性
	VOID RemovePropCache(ITEM& item);
	// 属性缓存数组
	int	m_PropMap[PI_PROP_COUNT];
	// 装备数组(_U_MAX个位置)
	ITEM m_Equipments[_U_MAX];
	// 空装备对象(用于返回)
	ITEM m_EmptyEquipment;
	// 错误信息字符串
	std::string m_strErrorMsg;
	// 所属玩家对象指针
	CHumanPlayer* m_pPlayer;
};