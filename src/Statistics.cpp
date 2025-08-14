#include "Statistics.h"

Statistics::Statistics(float lr):learningRate(lr) {}

void Statistics::nextInput() {
	lock_guard<mutex> lock(mtx);
	inputCount++;
}

int Statistics::getInput() {
	lock_guard<mutex> lock(mtx);
	return inputCount;
}
void Statistics::resetInput() {
	lock_guard<mutex> lock(mtx);
	inputCount=0;
}

void Statistics::setTotalInputs(int ti) {
	lock_guard<mutex> lock(mtx);
	totalInputs = ti;
}

int Statistics::getTotalInputs() {
	lock_guard<mutex> lock(mtx);
	return totalInputs;
}

void Statistics::resetEpoch() {
	lock_guard<mutex> lock(mtx);
	epochCount = 0;
}

void Statistics::nextEpoch(StopWatch& stopwatch) {
	lock_guard<mutex> lock(mtx);
	epochCount++;
	float time = stopwatch.elapsedMilliSeconds();
	epochTimes.load()->push_back(time);
}

int Statistics::getEpoch() {
	lock_guard<mutex> lock(mtx);
	return epochCount;
}

float Statistics::averageEpochTime() {
	//first time accidently writing /* */ comments needed to divide a pointer ref 
	//total/*epochTimes...
	float avg = epochTimes.load()->back() / epochTimes.load()->size();
	return avg;
}

float Statistics::lastEpochTime() {
	return epochTimes.load()->back();
}

void Statistics::setActivationType(string at) {
	lock_guard<mutex> lock(mtx);
	activationType.load()->assign(at);
}

string Statistics::getActivationType() {
	lock_guard<mutex> lock(mtx);
	return *activationType.load();
}


