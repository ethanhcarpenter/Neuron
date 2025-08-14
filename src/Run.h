#pragma once
#include "NN.h"
#include "DataSet.h"
#include "Rules.h"
using namespace std; 

struct RunParams {
	vector<int> architecture;
	bool visualiseNN;
	int epochs;
	float learningRate;
	float range;
	bool debugTimes;
	bool debugPredictions;
};

class Run {
private:
	RunParams runParams;
public:
	Run(RunParams rp);
	void runNumbers(int quality);
	void runRandomData(function<vector<float>(const vector<float>&)> rule, float noise);
};