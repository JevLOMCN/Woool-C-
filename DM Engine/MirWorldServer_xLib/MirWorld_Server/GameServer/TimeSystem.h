#pragma once
/**
 * 时间系统类
 * 继承自xSingletonClass, 实现单例模式
 * 用于管理游戏服务器的时间系统和时间事件
 */
class CTimeSystem : public xSingletonClass<CTimeSystem>
{
public:
	CTimeSystem(void);
	virtual ~CTimeSystem(void);
	// 更新时间系统状态
	VOID Update();
	/**
	 * 注册时间事件
	 * @param pTimeEvent 要注册的时间事件对象
	 * @return BOOL 注册是否成功
	 */
	BOOL RegisterTimeEvent(CTimeEventObject* pTimeEvent);
	/**
	 * 注销时间事件
	 * @param pTimeEvent 要注销的时间事件对象
	 * @return BOOL 注销是否成功
	 */
	BOOL UnRegisterTimeEvent(CTimeEventObject* pTimeEvent);
	// 获取当前时间
	WORD GetCurrentlyTime()const{ return m_wTime; }
	// 设置当前时间
	VOID SetCurrentlyTime(WORD wCurrentlyTime){ m_wTime = wCurrentlyTime; }
protected:
	// 秒变化时的处理
	VOID OnSecondChange();
	// 分钟变化时的处理
	VOID OnMinuteChange();
	// 小时变化时的处理
	VOID OnHourChange();
	// 日期变化时的处理
	VOID OnDayChange();
	// 月份变化时的处理
	VOID OnMonthChange();
	// 年份变化时的处理
	VOID OnYearChange();
private:
	CServerTimer m_tmrUpdate; // 更新计时器
	CSystemTime m_curTime; // 当前系统时间
	CSystemTime m_startupTime; // 系统启动时间
	xPtrQueue<CTimeEventObject> m_xTimeEventObjectQueue; // 时间事件对象队列
	WORD m_wTime; // 当前时间值
};