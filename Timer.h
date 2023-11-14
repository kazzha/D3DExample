#pragma once

namespace MyUtil
{
	class Timer
	{
	private:
		double mdDeltaTime;
		float mfScale;
		bool mbStopped;

		double mdSecondsPerCount;
		long long mllBaseTime;
		long long mllPausedTime;
		long long mllStopTime;
		long long mllPrevTime;
		long long mllCurrTime;


	public:
		Timer();

	public:
		void Start();
		void Stop();
		void Resume();
		void Update();

		float TotalTime() const;
		float DeltaTime() const;
		void SetScale(float scale);

	};

}

