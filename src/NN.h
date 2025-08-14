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
#include "Threader.h"
#include "Activations.h"

using namespace std;

enum NodeType {
	Input,
	Hidden,
	Output
};


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
	atomic <shared_ptr<Visualiser>*> visualiser;
	atomic <shared_ptr<Statistics>*> stats;
	atomic<shared_ptr<shared_mutex>*> statsMutex;
	
	Threader threader;
	bool visualise;
	vector<Layer> layers;
	vector<int> layerSizes;
	vector<vector<vector<float>>> weights;
	float learningRate;
	StopWatch stopwatch;
	string activationType;

public:
	NeuralNetwork();
	void setup(bool v, vector<int> ls, float lr, string at);

	void initWeights();

	void feedforward(vector<float>& inputVals, bool firstpass = false, bool updateWeights = false);

	void backpropagate(vector<float>& targetVals);

	vector<float> getOutput();
	vector<Layer>& getLayers();
	vector<vector<vector<float>>>& getWeights();
	int numberOfInputs();

	void train(DataSet& data, int epochs);
	void test(DataSet& data, float range);
	void threadVisualise();

	void shutdown();
};


