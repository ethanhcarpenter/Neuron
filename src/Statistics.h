#pragma once

#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <mutex>
#include <numeric>

#include "StopWatch.h"

using namespace std;

class Statistics {
private:
	mutex mtx;
	atomic<int> epochCount{ 0 };
	atomic<vector<float>*> epochTimes{ new vector<float>};
	atomic<int> inputCount{ 0 };
	atomic<int> totalInputs{ 0 };
	atomic<float> learningRate{ 0 };
	atomic<string*> activationType{ new string };
	atomic<float> error{ 0.0f };
	atomic<float> testAccuracy{ 0.0f };
public:
	Statistics(float lr);
	int getInput();
	void nextInput();
	void resetInput();
	int getEpoch();
	void nextEpoch(StopWatch& stopwatch);
	void resetEpoch();
	void setActivationType(string at);
	void setTotalInputs(int ti);
	int getTotalInputs();
	string getActivationType();
	float averageEpochTime();
	float lastEpochTime();

	
	

};