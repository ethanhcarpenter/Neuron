#include "NN.h"
#include "DataSet.h"
#include "Rules.cpp"

#include <numeric>
#include <iostream>
#include <cstdio>

int amain() {
	bool debugTimes = true;
	bool debugPredictions = false;
	bool visualiseNN = true;
	float range = 0.05;
	int epochs = 5000;
	float learningRate = 0.1;
	vector<int> architecture = { 14 * 14,20,1 };

	NeuralNetwork nn = { visualiseNN,architecture, learningRate };
	DataSet data = { nn.numberOfInputs() };

	data.generateImageDataFromTextFileRandom("data1414.txt", 200);
	nn.train(data, epochs, debugTimes);


	data.generateImageDataFromTextFileRandom("data1414.txt", 10000);
	nn.test(data, debugTimes, debugPredictions, range);


	nn.shutdown();
	return 0;
}
