#include "questevent.h"
#include "manager/questmanager.h"
#include "manager/userdatabase.h"

using namespace std;

CQuestEventBaseCondition::CQuestEventBaseCondition(CQuestEventTask* task, int eventType, int id, int goal)
{
	m_nEventType = eventType;
	m_pTask = task;
	m_nID = id;
	m_nGoalPoints = goal;

	m_pTask->AddCondition(this);
}

bool CQuestEventBaseCondition::Event_Internal(IUser* user)
{
	UserQuestTaskProgress progress = {};
	if (g_UserDatabase.GetQuestEventTaskProgress(user->GetID(), m_pTask->GetQuest()->GetID(), m_pTask->GetID(), progress) <= 0)
		return false;

	if (progress.unitsDone >= m_pTask->GetGoal())
		return false;

	return true;
}

int CQuestEventBaseCondition::GetID()
{
	return m_nID;
}

int CQuestEventBaseCondition::GetGoalPoints()
{
	return m_nGoalPoints;
}

int CQuestEventBaseCondition::GetEventType()
{
	return m_nEventType;
}

CQuestEventTask::CQuestEventTask(CQuestEvent* quest, int id, int goal)
{
	m_pQuest = quest;
	m_pQuest->AddTask(this);

	m_nID = id;
	m_nGoal = goal;
	m_nRewardID = 0;
	m_nNoticeGoal = 0;
}

CQuestEventTask::~CQuestEventTask()
{
	for (auto condition : m_Conditions)
	{
		delete condition;
	}
}

void CQuestEventTask::IncrementCount(IUser* user, int count, bool setForce)
{
	UserQuestTaskProgress progress = {};
	if (g_UserDatabase.GetQuestEventTaskProgress(user->GetID(), m_pQuest->GetID(), m_nID, progress) <= 0)
		return;

	if (count < 0)
		count = 1;

	// lol
	count ? setForce ? progress.unitsDone += count : progress.unitsDone = count : progress.unitsDone++;
	if (progress.unitsDone >= m_nGoal)
	{
		progress.unitsDone = m_nGoal;
		progress.finished = true;
	}

	if (g_UserDatabase.UpdateQuestEventTaskProgress(user->GetID(), m_pQuest->GetID(), progress) > 0)
	{
#ifdef _DEBUG
		Logger().Debug("[User '%s'] CQuestEventTask::IncrementCount: quest id: %d, task id: %d, done: %d, goal: %d\n", user->GetLogName(), m_pQuest->GetID(), m_nID, progress.unitsDone, m_nGoal);
#endif
		if (progress.unitsDone >= m_nGoal)
		{
			Done(user, progress);
		}
	}
}

void CQuestEventTask::Done(IUser* user, UserQuestTaskProgress& progress)
{
	m_pQuest->OnTaskDone(user, progress, this);
}

void CQuestEventTask::SetNotice(int goal, string userMsg)
{
	m_nNoticeGoal = goal;
	m_szNoticeUserMsg = userMsg;
}

void CQuestEventTask::SetRewardID(int rewardID)
{
	m_nRewardID = rewardID;
}

void CQuestEventTask::AddCondition(CQuestEventBaseCondition* condition)
{
	m_Conditions.push_back(condition);
}

void CQuestEventTask::ApplyProgress(IUser* user, UserQuestTaskProgress& progress)
{
	if (g_UserDatabase.UpdateQuestEventTaskProgress(user->GetID(), m_pQuest->GetID(), progress) <= 0)
		return;
}

void CQuestEventTask::OnMinuteTick(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_TIMEMATCH)
		{
			CQuestEventConditionTimeMatch* conditionTime = static_cast<CQuestEventConditionTimeMatch*>(condition);
			conditionTime->OnMinuteTick(userStat, gameMatch);
		}
	}
}

void CQuestEventTask::OnKillEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, GameMatch_KillEvent& killEvent)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_KILL)
		{
			CQuestEventConditionKill* conditionKill = static_cast<CQuestEventConditionKill*>(condition);
			conditionKill->OnKillEvent(userStat, gameMatch, killEvent);
		}
	}
}

void CQuestEventTask::OnBombExplode(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_BOMBEXPLODE)
		{
			CQuestEventConditionBombExplode* conditionExplode = static_cast<CQuestEventConditionBombExplode*>(condition);
			conditionExplode->OnBombExplode(userStat, gameMatch);
		}
	}
}

void CQuestEventTask::OnBombDefuse(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_BOMBDEFUSE)
		{
			CQuestEventConditionBombDefuse* conditionDefuse = static_cast<CQuestEventConditionBombDefuse*>(condition);
			conditionDefuse->OnBombDefuse(userStat, gameMatch);
		}
	}
}

void CQuestEventTask::OnHostageEscape(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_HOSTAGEESCAPE)
		{
			CQuestEventConditionHostageEscape* conditionHostageEscape = static_cast<CQuestEventConditionHostageEscape*>(condition);
			conditionHostageEscape->OnHostageEscape(userStat, gameMatch);
		}
	}
}

void CQuestEventTask::OnMonsterKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int monsterType)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_KILLMONSTER)
		{
			CQuestEventConditionKillMonster* conditionKillMonster = static_cast<CQuestEventConditionKillMonster*>(condition);
			conditionKillMonster->OnMonsterKill(userStat, gameMatch, monsterType);
		}
	}
}

void CQuestEventTask::OnMosquitoKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_KILLMOSQUITO)
		{
			CQuestEventConditionKillMosquito* conditionKillMosquito = static_cast<CQuestEventConditionKillMosquito*>(condition);
			conditionKillMosquito->OnMosquitoKill(userStat, gameMatch);
		}
	}
}

void CQuestEventTask::OnKiteKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_KILLKITE)
		{
			CQuestEventConditionKillKite* conditionKillKite = static_cast<CQuestEventConditionKillKite*>(condition);
			conditionKillKite->OnKiteKill(userStat, gameMatch);
		}
	}
}

void CQuestEventTask::OnLevelUpEvent(IUser* user, int level, int newLevel)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_LEVELUP)
		{
			CQuestEventConditionLevelUp* conditionLevelUP = static_cast<CQuestEventConditionLevelUp*>(condition);
			conditionLevelUP->OnLevelUpEvent(user, level, newLevel);
		}
	}
}

void CQuestEventTask::OnGameMatchLeave(IUser* user)
{

}

void CQuestEventTask::OnMatchEndEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int userTeam)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_MATCHWIN)
		{
			CQuestEventConditionWin* conditionMatchWin = static_cast<CQuestEventConditionWin*>(condition);
			conditionMatchWin->OnMatchEndEvent(userStat, gameMatch, userTeam);
		}
	}
}

void CQuestEventTask::OnUserLogin(IUser* user)
{
	for (auto condition : m_Conditions)
	{
		if (condition->GetEventType() == QuestTaskEventType::EVENT_LOGIN)
		{
			CQuestEventConditionLogin* conditionLogin = static_cast<CQuestEventConditionLogin*>(condition);
			conditionLogin->OnUserLogin(user);
		}
	}
}

/*std::string CQuestEventBaseTask::GetName()
{
	return m_szName;
}*/

int CQuestEventTask::GetID()
{
	return m_nID;
}

int CQuestEventTask::GetGoal()
{
	return m_nGoal;
}

int CQuestEventTask::GetNoticeGoal()
{
	return m_nNoticeGoal;
}

std::string CQuestEventTask::GetNoticeUserMsg()
{
	return m_szNoticeUserMsg;
}

int CQuestEventTask::GetRewardID()
{
	return m_nRewardID;
}

CQuestEvent* CQuestEventTask::GetQuest()
{
	return m_pQuest;
}

const vector<CQuestEventBaseCondition*>& CQuestEventTask::GetConditions()
{
	return m_Conditions;
}

bool CQuestEventTask::IsFinished(IUser* user)
{
	if (!g_UserDatabase.IsQuestEventTaskFinished(user->GetID(), m_pQuest->GetID(), m_nID))
		return false;

	return true;
}

CQuestEvent::CQuestEvent()
{
	m_nID = 0;
}

CQuestEvent::~CQuestEvent()
{
	for (auto task : m_Tasks)
	{
		delete task;
	}
}

void CQuestEvent::SetID(int id)
{
	m_nID = id;
}

int CQuestEvent::GetID()
{
	return m_nID;
}

void CQuestEvent::AddTask(CQuestEventTask* task)
{
	m_Tasks.push_back(task);
}

CQuestEventTask* CQuestEvent::GetTask(int id)
{
	auto taskIt = find_if(m_Tasks.begin(), m_Tasks.end(),
		[id](CQuestEventTask* task) { return id == task->GetID(); });

	if (taskIt != m_Tasks.end())
		return *taskIt;

	return NULL;
}

vector<CQuestEventTask*>& CQuestEvent::GetTasks()
{
	return m_Tasks;
}

void CQuestEvent::ApplyProgress(IUser* user, UserQuestProgress& progress)
{
	for (auto& taskProgress : progress.tasks)
	{
		CQuestEventTask* task = GetTask(taskProgress.taskID);
		if (task && !taskProgress.finished) // skip finished tasks
		{
			task->ApplyProgress(user, taskProgress);
		}
	}
}

void CQuestEvent::OnMinuteTick(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto task : m_Tasks)
	{
		task->OnMinuteTick(userStat, gameMatch);
	}
}

void CQuestEvent::OnKillEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, GameMatch_KillEvent& killEvent)
{
	for (auto task : m_Tasks)
	{
		task->OnKillEvent(userStat, gameMatch, killEvent);
	}
}

void CQuestEvent::OnBombExplode(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto task : m_Tasks)
	{
		task->OnBombExplode(userStat, gameMatch);
	}
}

void CQuestEvent::OnBombDefuse(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto task : m_Tasks)
	{
		task->OnBombDefuse(userStat, gameMatch);
	}
}

void CQuestEvent::OnHostageEscape(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto task : m_Tasks)
	{
		task->OnHostageEscape(userStat, gameMatch);
	}
}

void CQuestEvent::OnMonsterKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int monsterType)
{
	for (auto task : m_Tasks)
	{
		task->OnMonsterKill(userStat, gameMatch, monsterType);
	}
}

void CQuestEvent::OnMosquitoKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto task : m_Tasks)
	{
		task->OnMosquitoKill(userStat, gameMatch);
	}
}

void CQuestEvent::OnKiteKill(CGameMatchUserStat* userStat, CGameMatch* gameMatch)
{
	for (auto task : m_Tasks)
	{
		task->OnKiteKill(userStat, gameMatch);
	}
}

void CQuestEvent::OnLevelUpEvent(IUser* user, int level, int newLevel)
{
	for (auto task : m_Tasks)
	{
		task->OnLevelUpEvent(user, level, newLevel);
	}
}

void CQuestEvent::OnGameMatchLeave(IUser* user)
{

}

void CQuestEvent::OnMatchEndEvent(CGameMatchUserStat* userStat, CGameMatch* gameMatch, int userTeam)
{
	for (auto task : m_Tasks)
	{
		task->OnMatchEndEvent(userStat, gameMatch, userTeam);
	}
}

void CQuestEvent::OnUserLogin(IUser* user)
{
	for (auto task : m_Tasks)
	{
		task->OnUserLogin(user);
	}
}

void CQuestEvent::OnTaskDone(IUser* user, UserQuestTaskProgress& taskProgress, CQuestEventTask* doneTask)
{
	g_QuestManager.OnQuestEventTaskFinished(user, taskProgress, doneTask, this);
}

bool CQuestEvent::IsAllTaskFinished(IUser* user)
{
	for (auto task : m_Tasks)
	{
		if (!task->IsFinished(user))
			return false;
	}

	return true;
}