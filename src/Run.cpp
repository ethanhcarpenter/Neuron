#include "Run.h"

Run::Run(RunParams rp) :runParams(rp) {}
void Run::runNumbers(int quality) {
	NeuralNetwork nn = { runParams.visualiseNN,runParams.architecture, runParams.learningRate };
	DataSet data = { nn.numberOfInputs() };
	ostringstream oss;
	oss << "data" << quality << quality << ".txt";
	string filename = oss.str();

	data.generateImageDataFromTextFileRandom(filename.c_str(), 200);
	nn.train(data, runParams.epochs, runParams.debugTimes);

	data.generateImageDataFromTextFileRandom(filename.c_str(), 10000);
	nn.test(data, runParams.debugTimes, runParams.debugPredictions, runParams.range);

	nn.shutdown();
}

void Run::runRandomData(function<vector<float>(const vector<float>&)> rule, float noise) {
	NeuralNetwork nn = { runParams.visualiseNN,runParams.architecture, runParams.learningRate };
	DataSet data = { nn.numberOfInputs() };

	data.generateDataset(true, noise, rule);
	nn.train(data, runParams.epochs, runParams.debugTimes);

	data.generateDataset(false, noise, rule);
	nn.test(data, runParams.debugTimes, runParams.debugPredictions, runParams.range);

	nn.shutdown();
}