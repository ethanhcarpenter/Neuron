#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <random>
#include <shared_mutex>


#include "Visualiser.h"
#include "DataSet.h"
#include "Threader.h"
#include "Activations.h"




static float randomWeight(const std::string& activationType, int fanIn, int fanOut);


enum NodeType {
	Input,
	Hidden,
	Output
};

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
	std::vector<Node> nodes;

public:
	Layer(int size, NodeType type);
	std::vector<Node>& getNodes();
	int getSize();
};

struct VisualUpdate {
	int layer;
	int from;
	int to;
	float weight;
};

class NeuralNetwork {
	std::atomic <std::shared_ptr<Visualiser>*> visualiser;
	std::atomic <std::shared_ptr<Statistics>*> stats;
	std::atomic<std::shared_ptr<std::shared_mutex>*> statsMutex;
	Threader threader;
	std::vector<Layer> layers;
	std::vector<int> layerSizes;
	std::vector<std::vector<std::vector<float>>> weights;
	float learningRate;
	StopWatch stopwatch;
	std::string activationType;
	int maxEpochs;

public:
	NeuralNetwork();
	void setup();
	void initWeights();
	void feedforward(std::vector<float>& inputVals, bool firstpass = false, bool updateWeights = false);
	void backpropagate(std::vector<float>& targetVals);
	std::vector<float> getPredictedOutput();
	std::vector<std::vector<std::vector<float>>>& getWeights();
	void train(DataSet& data);
	void test(DataSet& data);
	void threadVisualise();
	void updateParametersFromVisualiser();
	void shutdown();
};


