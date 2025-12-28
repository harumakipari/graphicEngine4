#include "pch.h"
#include "Time.h"
#include <imgui.h>

Time::Time()
{
	LONGLONG counts_per_sec;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&counts_per_sec));
	secondsPerCount = 1.0 / static_cast<double>(counts_per_sec);

	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime));
	baseTime = thisTime;
	lastTime = thisTime;
}

void Time::Reset() // Call before message loop.
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime));
	baseTime = thisTime;
	lastTime = thisTime;

	stopTime = 0;
	stopped = false;
}

void Time::Start() // Call when unpaused.
{
	LONGLONG start_time;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start_time));

	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  baseTime       stopTime        start_time     
	if (stopped)
	{
		pausedTime += (start_time - stopTime);
		lastTime = start_time;
		stopTime = 0;
		stopped = false;
	}
}

void Time::Stop()
{
	if (!stopped)
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stopTime));
		stopped = true;
	}
}

void Time::Tick() // Call every frame.
{
	if (stopped)
	{
		deltaTime = 0.0;
		return;
	}

	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&thisTime));
	// Time difference between this frame and the previous.
	deltaTime = (thisTime - lastTime) * secondsPerCount * static_cast<double>(timeScale);
	unscaledDeltaTime = (thisTime - lastTime) * secondsPerCount;

	// Prepare for next frame.
	lastTime = thisTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (deltaTime < 0.0f)
	{
		deltaTime = 0.0f;
	}
}

float Time::TimeStamp() const  // in seconds
{
	// If we are stopped, do not count the time that has passed since we stopped.
	// Moreover, if we previously already had a pause, the distance 
	// stopTime - baseTime includes paused time, which we do not want to count.
	// To correct this, we can subtract the paused time from mStopTime:  
	//
	//                     |<--pausedTime-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  baseTime       stopTime        start_time     stopTime    thisTime

	if (stopped)
	{
		return static_cast<float>(((stopTime - pausedTime) - baseTime) * secondsPerCount);
	}

	// The distance thisTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from thisTime:  
	//
	//  (thisTime - pausedTime) - baseTime 
	//
	//                     |<--pausedTime-->|
	// ----*---------------*-----------------*------------*------> time
	//  baseTime       stopTime        start_time     thisTime
	else
	{
		return static_cast<float>(((thisTime - pausedTime) - baseTime) * secondsPerCount);
	}
}

void Time::DrawImGui()
{
#ifdef USE_IMGUI
	if (ImGui::Begin("Time"))
	{
		ImGui::Text("DeltaTime : %.4f", Time::DeltaTime());
		ImGui::Text("Unscaled : %.4f", Time::UnscaledDeltaTime());

		ImGui::Separator();

		ImGui::SliderFloat(
			"Time Scale",
			&Time::timeScale,
			0.0f,   // Š®‘S’âŽ~
			3.0f,   // 3”{‘¬
			"%.2f"
		);

		if (ImGui::Button("Reset TimeScale"))
		{
			Time::timeScale = 1.0f;
		}
	}
	ImGui::End();
#endif
}