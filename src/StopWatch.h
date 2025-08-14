#pragma once

#include <chrono>

using namespace std;

class StopWatch {
	chrono::time_point<chrono::high_resolution_clock> startTime;
public:
	void start();
	const float elapsedSeconds();
	const float elapsedMilliSeconds();
};
