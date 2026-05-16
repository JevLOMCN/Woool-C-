#pragma once
#include "findfile.h"

typedef struct task_step
{
	task_step()
	{
		FILLSELF(0);
	}
	BYTE btType; // 任务类型, 0 主线任务, 1 支线任务, 2 日常任务
	char* pszStepaim; // 任务介绍
	char* pszDesc; // 任务说明
}TASK_STEP;

typedef struct task_define
{
	task_define()
	{
		FILLSELF(0);
	}
	char* pszTitle;
	UINT nId;
	UINT nStepCount;
	TASK_STEP* pSteps;
}TASK_DEFINE;

class CTaskManager :
	public xSingletonClass<CTaskManager>,
	public CFindFile
{
public:
	CTaskManager(void);
	virtual ~CTaskManager(void);
	VOID Load(const char* pszPath);
	TASK_DEFINE* GetTaskDefine(UINT nId);
	TASK_STEP* GetTaskStep(UINT nId, UINT step);
	//获取任务类型, 0主线任务, 1为支线任务, 2日常任务
	int GetTaskType(const WORD wTaskId);
protected:
	TASK_DEFINE* m_pTaskMap[0xffff];
	VOID OnFoundFile(const char* pszFilename, UINT nParam = 0);
	xStringList<256> m_xTaskList;
	xObjectPool<TASK_DEFINE> m_xTaskPool;
};