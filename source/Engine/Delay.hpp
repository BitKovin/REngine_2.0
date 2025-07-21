#pragma once

#include "Time.hpp"

class Delay
{
public:
	Delay()
	{

	}

	Delay(double delay)
	{
		AddDelay(delay);
	}

	~Delay() 
	{

	}

	double waitUntilTime = -1000;

	double totalDuration = 0;

	bool ignorePause = false;

	bool Active = false;

	bool Wait()
	{
		double time = Time::GameTime;

		if (ignorePause)
		{
			time = Time::GameTimeNoPause;
		}

		return waitUntilTime >= time;

	}

	void AddDelay(double delay)
	{

		Active = true;

		double time = Time::GameTime;

		totalDuration = delay;

		if (ignorePause)
		{
			time = Time::GameTimeNoPause;
		}

		waitUntilTime = time + delay;

	}

	double GetRemainTime() const
	{
		double time = Time::GameTime;

		if (ignorePause)
		{
			time = Time::GameTimeNoPause;
		}

		return waitUntilTime - time;

	}

	float GetProgress()
	{
		return 1.0f -((totalDuration - GetRemainTime()) / totalDuration);
	}

private:

};