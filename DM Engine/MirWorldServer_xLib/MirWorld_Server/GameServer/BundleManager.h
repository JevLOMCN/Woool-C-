#pragma once

typedef struct tag_bundleinfo
{
	tag_bundleinfo()
	{
		FILLSELF(0);
	}
	char szName[20];
	char szExtractName[20];
	int	count;
}BUNDLEINFO;

//捆绑物品管理
class CBundleManager : public xSingletonClass<CBundleManager>
{
public:
	CBundleManager(void);
	virtual ~CBundleManager(void);
	//加载捆绑物品配置BundleItem.csv
	VOID LoadBundle(const char* pszBundleFile, BOOL bCSV);
	//获取捆绑物品信息
	BOOL GetBundleInfo(const char* pszName, char* pszExtractItemName, int& count);
private:
	CNameHash m_BundleNameHash;
};