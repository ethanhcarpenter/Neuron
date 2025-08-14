#include "StopWatch.h"


void StopWatch::start() {
	startTime = chrono::high_resolution_clock::now();
}

const float StopWatch::elapsedSeconds() {
	auto now = chrono::high_resolution_clock::now();
	chrono::duration<float> elapsed = now - startTime;
	return elapsed.count();
}
const float StopWatch::elapsedMilliSeconds() {
	auto now = chrono::high_resolution_clock::now();
	chrono::duration<float> elapsed = now - startTime;
	return elapsed.count() * 1000;
}
