#pragma once

class CWeatherSystem : public xSingletonClass<CWeatherSystem>, public CTimeEventObject
{
public:
	CWeatherSystem(void);
	virtual ~CWeatherSystem(void);
	VOID OnMinuteChange(CSystemTime& curTime);
};