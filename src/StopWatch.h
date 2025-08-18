#pragma once
#include <chrono>





class StopWatch {
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point pauseTime;
	std::chrono::duration<float> pausedDuration{ 0 };
	bool isPaused = false;
public:
	void start();
	void pause();
	void resume();
	float getElapsedSeconds() const;
	float getElapsedMilliSeconds()const;
};
