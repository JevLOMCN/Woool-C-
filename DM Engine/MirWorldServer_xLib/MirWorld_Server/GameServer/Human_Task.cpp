#include "stdafx.h"
#include "humanplayer.h"
#include "taskmanager.h"
#include "scriptvariable.h"

VOID CHumanPlayer::OnTaskInfo(TASKINFO* pInfo)
{
	this->m_TaskInfo = *pInfo;
	m_TaskIdToIndexMap.clear();
	for (int i = 0; i < MAX_TASK; i++)
	{
		if (m_TaskInfo.tasks[i].wTaskId > 0)
			m_TaskIdToIndexMap[m_TaskInfo.tasks[i].wTaskId] = i;
	}
	SendTaskInfo();
}

BOOL CHumanPlayer::AddTask(WORD wTaskId)
{
	if (HasTask(wTaskId)) return FALSE;

	int freepos = -1;
	for (int i = 0; i < MAX_TASK; i++)
	{
		if (this->m_TaskInfo.tasks[i].wTaskId == 0 && freepos < 0)freepos = i;
	}
	if (freepos < 0)return FALSE;

	// 初始化任务
	m_TaskInfo.tasks[freepos].wTaskId = wTaskId;
	m_TaskInfo.tasks[freepos].wStep = 1;
	memset(m_TaskInfo.tasks[freepos].vParam, 0, sizeof(m_TaskInfo.tasks[freepos].vParam));
	memset(m_TaskInfo.tasks[freepos].sParam, 0, sizeof(m_TaskInfo.tasks[freepos].sParam));
	m_TaskIdToIndexMap[wTaskId] = freepos;
	SendTaskInfo();
	return TRUE;
}

BOOL CHumanPlayer::DeleteTask(WORD wTaskId, BOOL bCompleteTask)
{
	auto it = m_TaskIdToIndexMap.find(wTaskId);
	if (it == m_TaskIdToIndexMap.end()) return FALSE;

	int i = it->second;
	this->m_TaskInfo.tasks[i].wTaskId = 0;
	this->m_TaskInfo.tasks[i].wStep = 0;
	RebuildTaskIdIndexMap(i);

	BYTE m = CTaskManager::GetInstance()->GetTaskType(wTaskId);//任务类型
	SendMsg(wTaskId, 0x9596, 0, 0, MAKEWORD(2, m));
	if (bCompleteTask)
	{
		m_TaskInfo.dwTotalTaskCount++; //已完成任务数+1
		m_TaskInfo.dwAchievement++; //积分+1
	}
	SendTaskInfo();
	return TRUE;
}

BOOL CHumanPlayer::UpdateTask(WORD wTaskId, WORD dwTaskStep)
{
	auto it = m_TaskIdToIndexMap.find(wTaskId);
	if (it == m_TaskIdToIndexMap.end()) return FALSE;

	m_TaskInfo.tasks[it->second].wStep = dwTaskStep;
	SendTaskInfo();
	return TRUE;
}

BOOL CHumanPlayer::HasTask(WORD wTaskId)
{
	if (wTaskId == 0)
	{
		for (int i = 0; i < MAX_TASK; i++)
		{
			if (m_TaskInfo.tasks[i].wTaskId > 0)
				return TRUE;
		}
		return FALSE;
	}
	return m_TaskIdToIndexMap.find(wTaskId) != m_TaskIdToIndexMap.end();
}

UINT CHumanPlayer::GetTaskStep(WORD wTaskId)
{
	auto it = m_TaskIdToIndexMap.find(wTaskId);
	if (it == m_TaskIdToIndexMap.end())
		return 0;
	return m_TaskInfo.tasks[it->second].wStep;
}

VOID CHumanPlayer::SetTaskValue(WORD wTaskId, BYTE btNum, DWORD dwData)
{
	auto it = m_TaskIdToIndexMap.find(wTaskId);
	if (it == m_TaskIdToIndexMap.end()) return;
	if (btNum > 9) return;

	m_TaskInfo.tasks[it->second].vParam[btNum] = dwData;
	SendTaskInfo();
}

int CHumanPlayer::GetTaskValue(WORD wTaskId, BYTE btNum)
{
	auto it = m_TaskIdToIndexMap.find(wTaskId);
	if (it == m_TaskIdToIndexMap.end()) return 0;
	if (btNum > 9) return 0;

	return m_TaskInfo.tasks[it->second].vParam[btNum];
}

VOID CHumanPlayer::SetTaskString(WORD wTaskId, BYTE btNum, const char* szpData)
{
	auto it = m_TaskIdToIndexMap.find(wTaskId);
	if (it == m_TaskIdToIndexMap.end()) return;
	if (btNum > 9) return;

	o_strncpy(m_TaskInfo.tasks[it->second].sParam[btNum], szpData, 32);
	SendTaskInfo();
}

const char* CHumanPlayer::GetTaskString(WORD wTaskId, BYTE btNum)
{
	auto it = m_TaskIdToIndexMap.find(wTaskId);
	if (it == m_TaskIdToIndexMap.end()) return "";
	if (btNum > 9) return "";

	return m_TaskInfo.tasks[it->second].sParam[btNum];
}

VOID CHumanPlayer::SendTaskInfo()
{
	char szTempBuffer[4096]{};
	//最大任务数、已完成任务数、跑环任务(当前第X环)、积分
	sprintf(szTempBuffer, "<TaskInfo MaxTaskCount=%u TotalTaskCount=%u LoopNum=0 Achievement=%u/>", MAX_TASK, m_TaskInfo.dwTotalTaskCount, m_TaskInfo.dwAchievement);
	SendMsg(0, 0x9596, 0, 0, 0, (LPVOID)szTempBuffer); // 发送任务概要信息

	CTaskManager* pTaskMgr = CTaskManager::GetInstance();
	for (int i = 0; i < MAX_TASK; i++)
	{
		WORD wTaskId = m_TaskInfo.tasks[i].wTaskId;
		if (wTaskId == 0) continue; // 如果Id等于0, 表示没有接这个任务
		WORD dwStep = m_TaskInfo.tasks[i].wStep;
		if (wTaskId > 0 && dwStep > 0)
		{
			TASK_DEFINE* pDefine = pTaskMgr->GetTaskDefine(wTaskId);
			if (pDefine == nullptr || dwStep > pDefine->nStepCount)
			{
				m_TaskInfo.tasks[i].wStep = 0;
				m_TaskInfo.tasks[i].wTaskId = 0;
				m_TaskIdToIndexMap.erase(wTaskId);
				continue;
			}

			char szTitle[128];
			char szTaskDesc[2048];
			char szStepaim[256];
			char szFinalTitle[128];
			char szFinalDesc[2048];
			char szFinalAim[256];
			// 读取任务模板中的数据, 并格式化一些变量
			ProcFmtText(pDefine->pszTitle, szTitle, 128, GetScriptTarget());
			TASK_STEP* pStep = pDefine->pSteps + (dwStep - 1);
			ProcFmtText(pStep->pszDesc, szTaskDesc, 2048, GetScriptTarget());
			if (pStep->pszStepaim)
				ProcFmtText(pStep->pszStepaim, szStepaim, 256, GetScriptTarget());
			else
				szStepaim[0] = '\0';
			// 使用玩家特定的任务参数进行解析
			ParseTaskParams(szTitle, szFinalTitle, 128, &m_TaskInfo.tasks[i]);
			ParseTaskParams(szTaskDesc, szFinalDesc, 2048, &m_TaskInfo.tasks[i]);
			ParseTaskParams(szStepaim, szFinalAim, 256, &m_TaskInfo.tasks[i]);

			if (szFinalAim[0] != '\0')
				sprintf(szTempBuffer, "<Task title=%s>%s</Task><aim>%s</aim>", szFinalTitle, szFinalDesc, szFinalAim);
			else
				sprintf(szTempBuffer, "<Task title=%s>%s</Task>", szFinalTitle, szFinalDesc);

			BYTE m = pTaskMgr->GetTaskType(wTaskId);//任务类型
			SendMsg(wTaskId, 0x9596, LOBYTE(dwStep), 0, MAKEWORD(1, m), (LPVOID)szTempBuffer);//添加新任务或更新现有任务信息
		}
	}
}

VOID CHumanPlayer::UpdateTaskToDB()
{
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
	if (pObj)
		pObj->UpdateTaskInfo(GetDBId(), &this->m_TaskInfo);
}

void CHumanPlayer::RebuildTaskIdIndexMap(int startIndex)
{
	m_TaskIdToIndexMap.clear();
	// 将 startIndex 之后的有效任务向前移动, 填补删除的空缺
	int ptr = startIndex;
	for (int j = startIndex + 1; j < MAX_TASK; j++)
	{
		if (m_TaskInfo.tasks[j].wTaskId > 0)
		{
			m_TaskInfo.tasks[ptr] = m_TaskInfo.tasks[j];
			m_TaskInfo.tasks[j].wStep = 0;
			m_TaskInfo.tasks[j].wTaskId = 0;
			m_TaskIdToIndexMap[m_TaskInfo.tasks[ptr].wTaskId] = ptr;
			ptr++;
		}
	}
	// 清空后续可能残留的任务
	for (int j = ptr; j < MAX_TASK; j++)
	{
		m_TaskInfo.tasks[j].wStep = 0;
		m_TaskInfo.tasks[j].wTaskId = 0;
	}
}

VOID CHumanPlayer::ParseTaskParams(const char* pszText, char* pszOutBuffer, int iOutBufferSize, TASKNODE* pTaskNode)
{
	const char* p = pszText;
	int targetptr = 0;
	iOutBufferSize -= 1;
	int textLen = (int)strlen(pszText);

	for (int i = 0; i < textLen && targetptr < iOutBufferSize; i++)
	{
		if (pszText[i] == '@' && i + 3 < textLen)
		{
			// 检查 @PS0-@PS2
			if (pszText[i + 1] == 'P' && pszText[i + 2] == 'S')
			{
				int index = pszText[i + 3] - '0';
				if (index >= 0 && index <= 9)
				{
					const char* param = pTaskNode->sParam[index];
					int len = (int)strlen(param);
					if (targetptr + len < iOutBufferSize)
					{
						memcpy(pszOutBuffer + targetptr, param, len);
						targetptr += len;
					}
					i += 3;
					continue;
				}
			}
			// 检查 @PI0-@PI2
			else if (pszText[i + 1] == 'P' && pszText[i + 2] == 'I')
			{
				int index = pszText[i + 3] - '0';
				if (index >= 0 && index <= 9)
				{
					int value = pTaskNode->vParam[index];
					int len = sprintf(pszOutBuffer + targetptr, "%d", value);
					if (targetptr + len < iOutBufferSize)
					{
						targetptr += len;
					}
					i += 3;
					continue;
				}
			}
		}
		pszOutBuffer[targetptr++] = pszText[i];
	}
	pszOutBuffer[targetptr] = 0;
}
