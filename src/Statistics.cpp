#include "Statistics.h"



#pragma region Initalise
Statistics::Statistics() {}
#pragma endregion



#pragma region Get
int Statistics::getInput() {
	std::lock_guard<std::mutex> lock(mtx);
	return inputCount;
}
int Statistics::getEpoch() {
	std::lock_guard<std::mutex> lock(mtx);
	return epochCount;
}
std::string Statistics::getActivationType() {
	std::lock_guard<std::mutex> lock(mtx);
	return *activationType.load();
}
float Statistics::getTestAccuracy() {
	return testAccuracy.load();
}
int Statistics::getTotalInputs() {
	std::lock_guard<std::mutex> lock(mtx);
	return totalInputs;
}
void Statistics::resetInput() {
	std::lock_guard<std::mutex> lock(mtx);
	inputCount = 0;
}
void Statistics::nextInput() {
	std::lock_guard<std::mutex> lock(mtx);
	inputCount++;
}
bool Statistics::getRunning() {
	return running.load();
}
int Statistics::getMaxEpochs() {
	return maxEpochs.load();
}
float Statistics::getLearningRate() {
	return learningRate.load();
}
bool Statistics::getNeuralNetworkNeedsUpdating() {
	return neuralNetworkNeedsUpdating.load();
}
std::vector<int> Statistics::getLayerSizes() {
	return *layerSizes.load();
}
std::vector<std::vector<std::vector<float>>> Statistics::getWeights() {
	return *weights.load();
}
float Statistics::getAverageEpochTime() {
	if (epochTimes.load()->size() == 0) { return 0; }
	float avg = epochTimes.load()->back() / epochTimes.load()->size();
	return avg;
}
float Statistics::getLastEpochTime() {
	if (epochTimes.load()->size() == 0) { return 0; }
	if (epochTimes.load()->size() == 1) { return epochTimes.load()->back(); }
	return (epochTimes.load()->at(epochTimes.load()->size() - 1) - epochTimes.load()->at(epochTimes.load()->size() - 2));
}
#pragma endregion



#pragma region Set
void Statistics::setTotalInputs(int ti) {
	std::lock_guard<std::mutex> lock(mtx);
	totalInputs = ti;
}
void Statistics::setActivationType(std::string at) {
	std::lock_guard<std::mutex> lock(mtx);
	activationType.load()->assign(at);
}
void Statistics::setTestAccuracy(float a) {
	std::lock_guard<std::mutex> lock(mtx);
	testAccuracy = a;
}
void Statistics::setRunning(bool r) {
	running = r;
}
void Statistics::setMaxEpochs(int e) {
	maxEpochs = e;
}
void Statistics::setWeights(const std::vector<std::vector<std::vector<float>>> w) {
	auto wPtr = weights.load();
	wPtr->clear();
	wPtr->resize(w.size());

	for (size_t i = 0; i < w.size(); i++) {
		(*wPtr)[i].resize(w[i].size());
		for (size_t j = 0; j < w[i].size(); j++) {
			(*wPtr)[i][j].resize(w[i][j].size());
			for (size_t k = 0; k < w[i][j].size(); k++) {
				(*wPtr)[i][j][k] = w[i][j][k];
			}
		}
	}
}
void Statistics::setLearningRate(float lr) {
	learningRate = lr;
}
void Statistics::setNeuralNetworkNeedsUpdating(bool nnnu) {
	neuralNetworkNeedsUpdating = nnnu;
}
void Statistics::setLayerSizes(std::vector<int> ls) {
	layerSizes.load()->clear();
	for (auto layer : ls) { layerSizes.load()->push_back(layer); }
}
#pragma endregion



#pragma region Logic
void Statistics::nextEpoch(StopWatch& stopwatch) {
	std::lock_guard<std::mutex> lock(mtx);
	epochCount++;
	float time = stopwatch.getElapsedMilliSeconds();
	epochTimes.load()->push_back(time);
}
void Statistics::resetEpoch() {
	std::lock_guard<std::mutex> lock(mtx);
	epochCount = 0;
}
#pragma endregion
