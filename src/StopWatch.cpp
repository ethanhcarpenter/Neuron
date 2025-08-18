#include "StopWatch.h"

#pragma region Logic
void StopWatch::start() {
	startTime = std::chrono::high_resolution_clock::now();
	pausedDuration = std::chrono::duration<float>::zero();
	isPaused = false;
}
void StopWatch::pause() {
	if (!isPaused) {
		pauseTime = std::chrono::high_resolution_clock::now();
		isPaused = true;
	}
}
void StopWatch::resume() {
	if (isPaused) {
		auto now = std::chrono::high_resolution_clock::now();
		pausedDuration += now - pauseTime;
		isPaused = false;
	}
}
#pragma endregion



#pragma region Get
float StopWatch::getElapsedSeconds() const {
	auto now = isPaused ? pauseTime : std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = now - startTime - pausedDuration;
	return elapsed.count();
}
float StopWatch::getElapsedMilliSeconds() const {
	return getElapsedSeconds() * 1000.0f;
}
#pragma endregion


