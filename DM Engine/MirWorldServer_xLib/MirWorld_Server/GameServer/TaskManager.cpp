#include "StdAfx.h"
#include ".\taskmanager.h"

CTaskManager::CTaskManager(void)
{
	memset(m_pTaskMap, 0, sizeof(m_pTaskMap));
}

CTaskManager::~CTaskManager(void)
{
}

VOID CTaskManager::Load(const char* pszPath)
{
	StartFind(pszPath, "*.txt", TRUE, 0);
}

VOID CTaskManager::OnFoundFile(const char* pszFilename, UINT nParam)
{
	CSettingFile sf;
	if (!sf.Open(pszFilename))return;
	TASK_DEFINE	define;
	define.nId = (UINT)sf.GetInteger("setup", "id", 0);
	if (define.nId == 0)return;
	if (define.nId > 65536)return;
	define.nStepCount = (UINT)sf.GetInteger("setup", "stepcount", 0);
	if (define.nStepCount == 0)return;
	define.pSteps = new TASK_STEP[define.nStepCount];
	if (define.pSteps == nullptr)return;
	define.pszTitle = copystring(sf.GetString("setup", "title", "未命名任务"));

	char szItemName[64];
	int n = 0;
	for (UINT i = 0; i < define.nStepCount; i++)
	{
		n = i + 1;
		define.pSteps[i].btType = sf.GetInteger("setup", "type", 0);
		sprintf(szItemName, "stepaim%u", n);
		define.pSteps[i].pszStepaim = copystring(sf.GetString("setup", szItemName, "没有任务描述"));
		sprintf(szItemName, "stepdesc%u", n);
		define.pSteps[i].pszDesc = copystring(sf.GetString("setup", szItemName, "没有任务描述"));
	}
	sf.Close();

	_splitpath(pszFilename, nullptr, nullptr, szItemName, nullptr);

	TASK_DEFINE* pDef = (TASK_DEFINE*)m_xTaskList.ObjectOf(szItemName);
	if (pDef == nullptr)
	{
		pDef = this->m_xTaskPool.newObject();
		if (pDef == nullptr)return;
		if (m_xTaskList.Add(szItemName, (LPVOID)pDef) == -1)
		{
			m_xTaskPool.deleteObject(pDef);
			return;
		}
		m_pTaskMap[define.nId - 1] = pDef;
	}
	else
	{
		PRINT(SUCCESS_GREEN, "任务 %s 被更新!\n", szItemName);
	}
	*pDef = define;
}

TASK_DEFINE* CTaskManager::GetTaskDefine(UINT nId)
{
	if (nId == 0 || nId > 65536)return nullptr;
	return m_pTaskMap[nId - 1];
}

TASK_STEP* CTaskManager::GetTaskStep(UINT nId, UINT step)
{
	TASK_DEFINE* p = GetTaskDefine(nId);
	if (p == nullptr)return nullptr;
	if (step == 0 || step > (int)p->nStepCount)return nullptr;
	return (p->pSteps + (step - 1));
}

int CTaskManager::GetTaskType(const WORD wTaskId)
{
	TASK_DEFINE* pDef = GetTaskDefine(wTaskId);
	if (pDef == nullptr || pDef->nStepCount == 0) return 0; //返回默认 主任务
	return pDef->pSteps[0].btType; // 获取任务类型
}