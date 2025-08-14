#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <atomic>




#include "Visualiser.h"
#include "DataSet.h"
#include "StopWatch.h"
#include "Progress.h"
#include "Threader.h"


using namespace std;

enum class NodeType { Input, Hidden, Output };

static float sigmoid(float x);

static float sigmoidDerivative(float x);

static float randomWeight();

class Node {
	NodeType type;
	float value;
	float bias;

public:
	Node(NodeType t);
	inline float getValue();
	void setValue(float v);
	inline float getBias();
	void setBias(float b);
	inline NodeType getType();
};

class Layer {
	vector<Node> nodes;

public:
	Layer(int size, NodeType type);
	vector<Node>& getNodes();
	int getSize();
};

class NeuralNetwork {
	std::atomic<Visualiser*> visualiser;
	Threader threader;
	Progress progress;
	bool visualise;
	vector<Layer> layers;
	vector<int> layerSizes;
	vector<vector<vector<float>>> weights;
	float learningRate;
	StopWatch stopwatch;
	vector<float> epochElapsedTimesMS;

public:
	NeuralNetwork(bool v, vector<int> ls, float lr = 0.1);

	void initWeights();

	void feedforward(vector<float>& inputVals, bool firstpass = false, bool updateWeights = false);

	void backpropagate(vector<float>& targetVals);

	vector<float> getOutput();
	vector<Layer>& getLayers();
	vector<vector<vector<float>>>& getWeights();
	int numberOfInputs();
	void outputEpochTimes();
	void outputTestTime();
	void outputPredictions(const vector<float>& predicted, const vector<float>& expected, float range);

	void train(DataSet& data, int epochs, bool debugTimes);
	void test(DataSet& data, bool debugTimes, bool debugPredictions, float range);
	void threadVisualise();

	void shutdown();
};


