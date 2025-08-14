#include "Run.h"

Run::Run(RunParams rp) :runParams(rp) {
	nn.setup (
		runParams.visualiseNN,
		runParams.architecture,
		runParams.learningRate,
		runParams.activationType
	);
}
void Run::runNumbers(int quality,int trainAmount,int testAmount) {
	
	DataSet data = { nn.numberOfInputs() };
	ostringstream oss;
	oss << "data\\data" << quality << quality << ".txt";
	string filename = oss.str();

	data.generateImageDataFromTextFileRandom(filename.c_str(), trainAmount);
	nn.train(data, runParams.epochs);

	data.generateImageDataFromTextFileRandom(filename.c_str(), testAmount);
	nn.test(data, runParams.range);

	nn.shutdown();
}

void Run::runRandomData(function<vector<float>(const vector<float>&)> rule, float noise) {

	DataSet data = { nn.numberOfInputs() };

	data.generateDataset(true, noise, rule);
	nn.train(data, runParams.epochs);

	data.generateDataset(false, noise, rule);
	nn.test(data, runParams.range);

	nn.shutdown();
}