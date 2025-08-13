#include "NN.h"
#include "DataSet.h"
#include "Rules.cpp"

#include <numeric>
#include <iostream>
#include <cstdio>

int main() {
	bool debugTimes = true;
	bool debugPredictions = false;
	bool visualiseNN = true;
	float range = 0.05;
	int epochs = 1000;
	float learningRate = 0.1;
	vector<int> architecture = { 10,100,100,100,1 };
	auto rule = RuleBook::sumGreaterThanN(8);


	NeuralNetwork nn = { visualiseNN,architecture, learningRate };
	DataSet data = { nn.numberOfInputs() };

	data.generateDataset(0.05, rule);
	nn.train(data, epochs, debugTimes);


	data.generateDataset(0.05, rule);
	nn.test(data, debugTimes, debugPredictions, range);


	nn.shutdown();
	return 0;
}
