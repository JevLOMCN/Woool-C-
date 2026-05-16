#include "StdAfx.h"
#include ".\bundlemanager.h"
#include "fmttextfile.h"

CBundleManager::CBundleManager(void)
{
}

CBundleManager::~CBundleManager(void)
{
}

VOID CBundleManager::LoadBundle(const char* pszBundleFile, BOOL bCSV)
{
	CFmtTextFile ftfBundle("s20s20d", pszBundleFile, bCSV);
	BUNDLEINFO bundleinfo, * pBundleInfo = nullptr;
	for (int i = 0; i < ftfBundle.GetCount(); i++)
	{
		if (ftfBundle.GetStruct(i, &bundleinfo))
		{
			pBundleInfo = new BUNDLEINFO;
			*pBundleInfo = bundleinfo;
			if (!m_BundleNameHash.HAdd(bundleinfo.szName, pBundleInfo))
			{
				delete pBundleInfo;
				pBundleInfo = (BUNDLEINFO*)m_BundleNameHash.HGet(bundleinfo.szName);
				if (pBundleInfo)
				{
					if (memcmp(pBundleInfo, &bundleinfo, sizeof(bundleinfo)) != 0)
						*pBundleInfo = bundleinfo;
				}
			}
		}
	}
}

BOOL CBundleManager::GetBundleInfo(const char* pszName, char* pszExtractItemName, int& count)
{
	BUNDLEINFO* pInfo = (BUNDLEINFO*)m_BundleNameHash.HGet(pszName);
	if (pInfo)
	{
		o_strncpy(pszExtractItemName, pInfo->szExtractName, 19);
		count = pInfo->count;
		return TRUE;
	}
	return FALSE;
}