/* 

Amine Rehioui
Created: August 7th 2011

*/

#include "Shoot.h"

#include "Timer.h"
#include "TimeManager.h"

namespace shoot
{
	//! constructor
	Timer::Timer()
		: m_fDuration(0.0f)
		, m_fTimeLeft(0.0f)
		, m_bRunning(false)
	{
		TimeManager::Instance()->RegisterTimer(this);
	}

	//! destructor
	Timer::~Timer()
	{
		TimeManager::Instance()->UnregisterTimer(this);
	}

	//! starts the timer
	void Timer::Start(f32 fDuration)
	{
		m_fDuration = fDuration;
		m_fTimeLeft = m_fDuration;
		m_bRunning = true;
	}

	//! stops the timer
	void Timer::Stop()
	{
		m_bRunning = false;
	}

	//! returns the progress ratio
	f32 Timer::GetProgressRatio() const
	{
		return (1.0f - (m_fTimeLeft/m_fDuration));
	}

	//! returns the progress duration
	f32 Timer::GetProgressDuration() const
	{
		return (m_fDuration - m_fTimeLeft);
	}

	//! advances the timer
	void Timer::Advance(f32 fDeltaTime)
	{
		if(m_bRunning)
		{
			m_fTimeLeft -= fDeltaTime;
			if(m_fTimeLeft < 0.0f)
			{
				m_fTimeLeft = 0.0f;
				m_bRunning = false;
			}
		}
	}
}
