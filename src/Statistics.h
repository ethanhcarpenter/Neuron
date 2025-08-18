#pragma once
#include <atomic>
#include <vector>
#include <string>
#include <mutex>



#include "StopWatch.h"



class Statistics {
private:
	std::mutex mtx;
	std::atomic<int> maxEpochs{ 0 };
	std::atomic<int> epochCount{ 0 };
	std::atomic<std::vector<float>*> epochTimes{ new std::vector<float> };
	std::atomic<int> inputCount{ 0 };
	std::atomic<int> totalInputs{ 0 };
	std::atomic<float> learningRate{ 0 };
	std::atomic<std::string*> activationType{ new std::string };
	std::atomic<float> error{ 0.0f };
	std::atomic<float> testAccuracy{ 0.0f };
	std::atomic<bool> running{ false };
	std::atomic<std::vector<int>*>layerSizes{ new std::vector<int> };
	std::atomic<std::vector<std::vector<std::vector<float>>>*> weights{ new std::vector<std::vector<std::vector<float>>> };
	std::atomic<bool> neuralNetworkNeedsUpdating{ false };
public:
	Statistics();
	int getInput();
	void nextInput();
	void resetInput();
	int getEpoch();
	void nextEpoch(StopWatch& stopwatch);
	void resetEpoch();
	void setActivationType(std::string at);
	void setTotalInputs(int ti);
	int getTotalInputs();
	std::string getActivationType();
	float getAverageEpochTime();
	float getLastEpochTime();
	float getTestAccuracy();
	void setTestAccuracy(float a);
	void setRunning(bool r);
	bool getRunning();
	void setWeights(std::vector<std::vector<std::vector<float>>> w);
	std::vector<std::vector<std::vector<float>>> getWeights();
	void setMaxEpochs(int e);
	int getMaxEpochs();
	void setLearningRate(float lr);
	float getLearningRate();
	void setLayerSizes(std::vector<int> ls);
	std::vector<int> getLayerSizes();
	bool getNeuralNetworkNeedsUpdating();
	void setNeuralNetworkNeedsUpdating(bool nnnu);



};