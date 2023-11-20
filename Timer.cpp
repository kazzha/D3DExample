#include <Windows.h>
#include "Timer.h"

using namespace MyUtil;

Timer::Timer() :
	mdDeltaTime { -1.0 },
	mfScale { 1.0f },
	mbStopped { false },
	mdSecondsPerCount { 0.0 },
	mllBaseTime{ 0 },
	mllPausedTime{ 0 },
	mllStopTime{ 0 },
	mllPrevTime{ 0 },
	mllCurrTime{ 0 }
{
	long long countsPerSec;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));

	mdSecondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

void Timer::Start()
{
	long long currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mllBaseTime = currTime;
	mllPrevTime = currTime;
	mllStopTime = 0;
	mbStopped = false;

}

void Timer::Stop()
{
	if (!mbStopped)
	{
		long long currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mllStopTime = currTime;
		mbStopped = true;
	}
}

void Timer::Resume()
{
	if (mbStopped)
	{
		//¸ØÃá ½Ã°£: stoptime, ¸Þ´º ¿¬ ½Ã°£: currtime -> curr-stop¸¦ ÇÏ¸é pausetimeÀÌ ³ª¿È
		long long currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mllPausedTime += (currTime - mllStopTime); 
		mllPrevTime = currTime;
		mllStopTime = 0;
		mbStopped = false;
	}
}

void Timer::Update()
{
	if (mbStopped)
	{
		mdDeltaTime = 0.0;
		return;
	}

	long long currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mllCurrTime = currTime;

	mdDeltaTime = (mllCurrTime - mllPrevTime) * mdSecondsPerCount;

	mllPrevTime = mllCurrTime;

	if (mdDeltaTime < 0.0)
	{
		mdDeltaTime = 0.0;
	}
	
}

float Timer::TotalTime() const
{
	if (mbStopped)
	{
		return static_cast<float>((mllStopTime - mllBaseTime - mllPausedTime) * mdSecondsPerCount);
	}
	else
	{
		return static_cast<float>((mllCurrTime - mllBaseTime - mllPausedTime) * mdSecondsPerCount);
	}
}

float Timer::DeltaTime() const
{
	return static_cast<float>(mdDeltaTime * mfScale);
}

void Timer::SetScale(float scale)
{
	mfScale = scale;

}
