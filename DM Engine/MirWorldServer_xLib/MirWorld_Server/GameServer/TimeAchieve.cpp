#include "stdafx.h"
#include "TimeAchieve.h"
#include "tinyxml.h"
#include "HumanPlayer.h"

CTimeAchieve::CTimeAchieve(void)
{
}

CTimeAchieve::~CTimeAchieve(void)
{
	m_mapShowProps.clear();
	m_vecLevels.clear();
	m_vecAchieves.clear();
	m_mapAchieveIdToIndex.clear();
	m_mapGroupToIndex.clear();
}

VOID CTimeAchieve::Load(const char* pszFilename)
{
	TiXmlDocument doc(pszFilename);
	if (!doc.LoadFile(TIXML_ENCODING_UNKNOWN))
	{
		PRINT(ERROR_RED, "加载 TimeAchieve.xml 文件失败: %s \n", pszFilename);
		return;
	}

	TiXmlElement* root = doc.FirstChildElement("TimeAchieve");
	if (root == nullptr)
	{
		PRINT(ERROR_RED, "TimeAchieve.xml 文件格式错误: 找不到 TimeAchieve 根节点\n");
		return;
	}
	unsigned int temp; //定义缓存值，用于下面转换

	// 1. 加载 JobShowProps
	for (TiXmlElement* jobShowElem = root->FirstChildElement("JobShowProps"); jobShowElem != nullptr; jobShowElem = jobShowElem->NextSiblingElement("JobShowProps"))
	{
		int nJob = -1;
		jobShowElem->QueryIntAttribute("job", &nJob);
		if (nJob < 0) continue;

		std::vector<TIMEACHIEVE_SHOWPROP> vecShowProps;
		for (TiXmlElement* propElem = jobShowElem->FirstChildElement("ShowProp"); propElem != nullptr; propElem = propElem->NextSiblingElement("ShowProp"))
		{
			TIMEACHIEVE_SHOWPROP showProp;
			memset(&showProp, 0, sizeof(showProp));

			const char* pszName = propElem->Attribute("name");
			if (pszName != nullptr)
			{
				o_strncpy(showProp.szName, pszName, 7);
			}

			const char* pszShowName = propElem->Attribute("show_name");
			if (pszShowName != nullptr)
			{
				o_strncpy(showProp.szShowName, pszShowName, 7);
			}

			vecShowProps.push_back(showProp);
		}

		if (!vecShowProps.empty())
		{
			m_mapShowProps[nJob] = vecShowProps;
		}
	}
	
	// 2. 加载 AchieveLevel
	for (TiXmlElement* levelElem = root->FirstChildElement("AchieveLevel"); levelElem != nullptr; levelElem = levelElem->NextSiblingElement("AchieveLevel"))
	{
		TIMEACHIEVE_LEVEL level;
		levelElem->QueryUnsignedAttribute("max_exp", &temp);
		level.nMaxExp = (DWORD)temp;
		for (TiXmlElement* jobPropElem = levelElem->FirstChildElement("Jobprop"); jobPropElem != nullptr; jobPropElem = jobPropElem->NextSiblingElement("Jobprop"))
		{
			int nJob = -1;
			jobPropElem->QueryIntAttribute("job", &nJob);
			if (nJob < 0) continue;

			std::map<std::string, TIMEACHIEVE_PROPVALUE> mapProps;
			for (TiXmlElement* propElem = jobPropElem->FirstChildElement("Prop"); propElem != nullptr; propElem = propElem->NextSiblingElement("Prop"))
			{
				const char* pszName = propElem->Attribute("name");
				if (pszName == nullptr) continue;

				TIMEACHIEVE_PROPVALUE propValue;
				propElem->QueryUnsignedAttribute("value_1", &temp);
				propValue.nValue1 = (BYTE)temp;
				propElem->QueryUnsignedAttribute("value_2", &temp);
				propValue.nValue2 = (BYTE)temp;

				mapProps[pszName] = propValue;
			}

			if (!mapProps.empty())
			{
				level.mapJobProps[nJob] = mapProps;
			}
		}

		m_vecLevels.push_back(level);
	}

	// 3. 加载 Achieve (单个成就项)
	for (TiXmlElement* achieveElem = root->FirstChildElement("Achieve"); achieveElem != nullptr; achieveElem = achieveElem->NextSiblingElement("Achieve"))
	{
		TIMEACHIEVE_ITEM achieve;
		memset(&achieve, 0, sizeof(achieve));
		achieveElem->QueryUnsignedAttribute("id", &temp);
		achieve.nId = (WORD)temp;
		achieveElem->QueryUnsignedAttribute("max_exp", &temp);
		achieve.nMaxExp = (DWORD)temp;
		achieveElem->QueryUnsignedAttribute("group", &temp);
		achieve.nGroup = (BYTE)temp;
		achieveElem->QueryUnsignedAttribute("point", &temp);
		achieve.nPoint = (WORD)temp;

		const char* pszName = achieveElem->Attribute("name");
		if (pszName != nullptr)
		{
			o_strncpy(achieve.szName, pszName, 31);
		}

		m_vecAchieves.push_back(achieve);
		int nIndex = (int)m_vecAchieves.size() - 1;
		m_mapAchieveIdToIndex[achieve.nId] = nIndex;
		m_mapGroupToIndex[achieve.nGroup].push_back(nIndex);
	}
}

const std::vector<TIMEACHIEVE_SHOWPROP>* CTimeAchieve::GetShowProps(int nJob) const
{
	auto it = m_mapShowProps.find(nJob);
	if (it != m_mapShowProps.end())
	{
		return &it->second;
	}
	return nullptr;
}

const TIMEACHIEVE_LEVEL* CTimeAchieve::GetLevel(int nLevel) const
{
	if (nLevel >= 0 && nLevel < (int)m_vecLevels.size())
	{
		return &m_vecLevels[nLevel];
	}
	return nullptr;
}

const std::map<std::string, TIMEACHIEVE_PROPVALUE>* CTimeAchieve::GetJobProps(int nLevel, int nJob) const
{
	const TIMEACHIEVE_LEVEL* pLevel = GetLevel(nLevel);
	if (pLevel == nullptr) return nullptr;

	auto it = pLevel->mapJobProps.find(nJob);
	if (it != pLevel->mapJobProps.end())
		return &it->second;
	return nullptr;
}

BOOL CTimeAchieve::GetPropValue(int nLevel, int nJob, const char* pszPropName, TIMEACHIEVE_PROPVALUE& outValue) const
{
	const std::map<std::string, TIMEACHIEVE_PROPVALUE>* pProps = GetJobProps(nLevel, nJob);
	if (pProps == nullptr || pszPropName == nullptr) return FALSE;

	auto it = pProps->find(pszPropName);
	if (it != pProps->end())
	{
		outValue = it->second;
		return TRUE;
	}
	return FALSE;
}

DWORD CTimeAchieve::GetLevelMaxExp(int nLevel) const
{
	const TIMEACHIEVE_LEVEL* pLevel = GetLevel(nLevel);
	if (pLevel != nullptr)
		return pLevel->nMaxExp;
	return -1;
}

const TIMEACHIEVE_ITEM* CTimeAchieve::GetAchieve(int nIndex) const
{
	if (nIndex >= 0 && nIndex < (int)m_vecAchieves.size())
		return &m_vecAchieves[nIndex];
	return nullptr;
}

const int CTimeAchieve::FindIndexById(int nId) const
{
	auto it = m_mapAchieveIdToIndex.find(nId);
	if (it != m_mapAchieveIdToIndex.end())
		return it->second;
	return -1;
}

const TIMEACHIEVE_ITEM* CTimeAchieve::FindAchieveById(int nId) const
{
	auto it = m_mapAchieveIdToIndex.find(nId);
	if (it != m_mapAchieveIdToIndex.end())
		return GetAchieve(it->second);
	return nullptr;
}

const std::vector<int>* CTimeAchieve::FindAchieveGroupById(int nId) const
{
	auto it = m_mapGroupToIndex.find(nId);
	if (it != m_mapGroupToIndex.end())
		return &it->second;
	return nullptr;
}

static char g_szTempBuffer[8192];
VOID CTimeAchieve::SendAchieveData(CHumanPlayer* pPlayer)
{
	xPacket packet(g_szTempBuffer, 8192);
	
	packet.push(1);//未知
	DWORD nCurExp = pPlayer->GetAchieveExp(); //玩家-当前成就经验值
	packet.push(&nCurExp, 4);//当前经验值

	// 成就ID列表
	DWORD nAchieveCount = (DWORD)m_mapAchieveIdToIndex.size();
	packet.push(&nAchieveCount, 4);
	for (const auto& pair : m_mapAchieveIdToIndex)
	{
		int nId = pair.first;
		packet.push(&nId, 4);
	}

	//玩家的成就数据
	for (int i = 0; i < 3; ++i)
	{
		packet.push(&nAchieveCount, 4);
		pPlayer->PacketAchieve(packet, i, nAchieveCount);
	}

	BYTE btJob = pPlayer->GetPro();
	// 获取职业显示属性
	auto itShow = m_mapShowProps.find(btJob);
	const auto& vecShow = itShow->second;

	// 当前
	size_t nLevel = pPlayer->GetAchieveLevel();
	PushPropBlock(packet, btJob, vecShow, m_vecLevels[nLevel]);

	packet.push(3);  // 未知
	packet.push(&nLevel, 4);  // 玩家-成就等级
	DWORD nMaxExp = (DWORD)m_vecLevels[0].nMaxExp;
	packet.push(&nMaxExp, 4); // 当前最大经验值

	// 下级
	if (m_vecLevels.size() > nLevel)
		PushPropBlock(packet, btJob, vecShow, m_vecLevels[nLevel + 1]);
	else
		PushPropBlock(packet, btJob, vecShow, m_vecLevels[nLevel]);
	pPlayer->SendMsg(pPlayer->GetId(), 0x959, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
}