#pragma once
enum match_state
{
	MS_MATCHGROUP,
	MS_NEXTGROUP,
};

//套装触发管理
class CSpecialEquipmentManager : public xSingletonClass<CSpecialEquipmentManager>
{
public:
	CSpecialEquipmentManager(void);
	virtual ~CSpecialEquipmentManager(void);
	//加载套装触发SpecialItem.txt
	VOID LoadSpecialEquipmentFunction(const char* pszFilename);
	//匹配套装
	BOOL MatchFunction(CHumanPlayer* pPlayer, special_equipment_func func);
	//匹配套装
	BOOL MatchFunction(CHumanPlayer* pPlayer, special_equipment_func func, DWORD& dwPosFlag);
	//获取指定套装定义
	char* GetFunctionString(special_equipment_func func)const
	{
		if (func < 0 || func >= SEF_MAX)return nullptr;
		return m_SpecialFunctionDefine[func].pszSpecial;
	}
	//获取指定套装参数
	DWORD GetFunctionParam(special_equipment_func func, int paramindex)const
	{
		if (func < 0 || func >= SEF_MAX)return 0;
		if (paramindex < 0 || paramindex >= m_SpecialFunctionDefine[func].paramcount)return 0;
		return m_SpecialFunctionDefine[func].pParams[paramindex];
	}
protected:
	SpecialEquipmentFunction m_SpecialFunctionDefine[SEF_MAX];
	SpecialEquipment* m_pSpecialEquipmentArray;
	int	m_iSpecialEquipmentCount;
};