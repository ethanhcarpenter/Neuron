#include "NN.h"

static float sigmoid(float x) {
	return 1.0 / (1.0 + exp(-x));
}

static float sigmoidDerivative(float x) {
	return x * (1 - x);
}

static float randomWeight() {
	return ((float)rand() / RAND_MAX) * 2 - 1;
}



Node::Node(NodeType t) : type(t), value(0), bias(randomWeight()) {}
float Node::getValue() { return value; }
void Node::setValue(float v) { value = v; }
float Node::getBias() { return bias; }
void Node::setBias(float b) { bias = b; }
NodeType Node::getType() { return type; }



Layer::Layer(int size, NodeType type) {
	for (int i = 0; i < size; ++i)
		nodes.emplace_back(type);
}
vector<Node>& Layer::getNodes() { return nodes; }
int Layer::getSize() { return nodes.size(); }



NeuralNetwork::NeuralNetwork(bool v, vector<int> ls, float lr) {
	learningRate = lr;
	visualise = v;
	layerSizes = ls;
	int index = 0;
	for (int size : layerSizes) {
		NodeType type = (index == 0 ? NodeType::Input :
			(index == layerSizes.size() - 1 ? NodeType::Output : NodeType::Hidden));
		layers.emplace_back(size, type);
		index++;
	}
	initWeights();
	threader.getSide() = thread(&Threader::visualiserWorker, &threader);
	if (visualise) {
		visualiser.store(new Visualiser);
	}

}

void NeuralNetwork::initWeights() {
	weights.resize(layers.size() - 1);
	for (size_t l = 0; l < layers.size() - 1; ++l) {
		int fromSize = layers[l].getSize();
		int toSize = layers[l + 1].getSize();
		weights[l].resize(fromSize);
		for (int i = 0; i < fromSize; ++i) {
			weights[l][i].resize(toSize);
			for (int j = 0; j < toSize; ++j) {
				weights[l][i][j] = randomWeight();
			}
		}
	}
}

void NeuralNetwork::feedforward(vector<float>& inputVals, bool firstPass, bool updateWeights) {
	auto& inputLayer = layers[0].getNodes();
	for (int i = 0; i < inputVals.size(); ++i)
		inputLayer[i].setValue(inputVals[i]);

	if (!visualise) {
		for (size_t l = 1; l < layers.size(); ++l) {
			auto& prev = layers[l - 1].getNodes();
			auto& curr = layers[l].getNodes();
			for (int j = 0; j < curr.size(); ++j) {
				float sum = 0;
				for (int i = 0; i < prev.size(); ++i)
					sum += prev[i].getValue() * weights[l - 1][i][j];
				sum += curr[j].getBias();
				curr[j].setValue(sigmoid(sum));
			}
		}
		return;
	}

	struct VisualUpdate {
		int layer, from, to;
		float weight;
	};
	std::vector<VisualUpdate> updates;

	for (size_t l = 1; l < layers.size(); ++l) {
		auto& prev = layers[l - 1].getNodes();
		auto& curr = layers[l].getNodes();
		for (int j = 0; j < curr.size(); ++j) {
			float sum = 0;
			for (int i = 0; i < prev.size(); ++i) {
				sum += prev[i].getValue() * weights[l - 1][i][j];
				updates.push_back({ int(l - 1), i, j, weights[l - 1][i][j] });
			}
			sum += curr[j].getBias();
			curr[j].setValue(sigmoid(sum));
		}
	}

	if (!updateWeights) { return; }
	const int MAX_QUEUE_SIZE = 1600;
	std::unique_lock<std::mutex> lock(threader.getQueueMutex());
	threader.getQueue().emplace([updates = move(updates), firstPass, this, visualiser = visualiser.load()]() {
		if (!visualiser) return;
		int update = 0;
		for (auto& u : updates) {
			int layer = u.layer;
			int from = u.from;
			int to = u.to;
			float weight = u.weight;
			float lastweight = 11;
			if (!firstPass) lastweight = visualiser->getConnectionWeight(layer, from, to, weight);
			if (roundf(abs(lastweight) * 10) == roundf(abs(weight) * 10)) continue;
			visualiser->updateConnection(layer, from, to, weight);
		}
		});

	threader.getCV().notify_one();
}


void NeuralNetwork::backpropagate(vector<float>& targetVals) {
	vector<vector<float>> deltas(layers.size());
	auto& output = layers.back().getNodes();
	const int outputSize = output.size();
	deltas.back().resize(outputSize);
	for (int i = 0; i < outputSize; ++i) {
		float outVal = output[i].getValue();
		float error = targetVals[i] - outVal;
		deltas.back()[i] = error * sigmoidDerivative(outVal);
	}

	for (int l = layers.size() - 2; l > 0; --l) {
		auto& curr = layers[l].getNodes();
		auto& next = layers[l + 1].getNodes();
		const int currSize = curr.size();
		const int nextSize = next.size();
		deltas[l].resize(currSize);
		for (int i = 0; i < currSize; ++i) {
			float sum = 0;
			for (int j = 0; j < nextSize; ++j) {
				sum += weights[l][i][j] * deltas[l + 1][j];
			}

			deltas[l][i] = sum * sigmoidDerivative(curr[i].getValue());
		}
	}

	for (size_t l = 0; l < weights.size(); ++l) {
		auto& from = layers[l].getNodes();
		auto& to = layers[l + 1].getNodes();
		const int fromSize = from.size();
		const int toSize = to.size();
		for (int i = 0; i < fromSize; ++i) {
			float fromVal = from[i].getValue();
			for (int j = 0; j < toSize; ++j) {
				weights[l][i][j] += learningRate * deltas[l + 1][j] * fromVal;
			}
		}
		for (int j = 0; j < toSize; ++j) {
			float newBias = to[j].getBias() + learningRate * deltas[l + 1][j];
			to[j].setBias(newBias);
		}
	}
}

vector<float> NeuralNetwork::getOutput() {
	vector<float> output;
	for (auto& node : layers.back().getNodes())
		output.push_back(node.getValue());
	return output;
}

vector<Layer>& NeuralNetwork::getLayers() {
	return layers;
}
vector<vector<vector<float>>>& NeuralNetwork::getWeights() {
	return weights;
}

int NeuralNetwork::numberOfInputs() {
	return layers[0].getNodes().size();
}
void NeuralNetwork::outputEpochTimes() {
	float total = stopwatch.elapsedSeconds();
	float sum = epochElapsedTimesMS[0];
	for (int i = 0; i < epochElapsedTimesMS.size() - 1; i++) { sum += (epochElapsedTimesMS[i + 1] - epochElapsedTimesMS[i]); }
	float avg = sum / epochElapsedTimesMS.size();
	cout << "[EPOCH TIMES] average epoch time: " << avg << " ms (" << epochElapsedTimesMS.size() << " epochs)" << "\n";
	cout << "[EPOCH TIMES] total training time: " << total << " s " << "\n";
}
void NeuralNetwork::outputTestTime() {
	float total = stopwatch.elapsedSeconds();
	cout << "[TEST TIMES] total test time: " << total << " s " << "\n";
}

void NeuralNetwork::outputPredictions(const vector<float>& predicted, const vector<float>& expected, float range) {
	for (size_t j = 0; j < predicted.size(); ++j) {
		bool correct = abs(predicted[j] - expected[j]) < range;
		cout << (correct ? "[PASS]" : "[FAIL]")
			<< " p:" << predicted[j]
			<< " e:" << expected[j];
		if (j < predicted.size() - 1) cout << " | ";
	}
	cout << "\n";
}

void NeuralNetwork::threadVisualise() {
	threader.getMain() = thread([&, this] {
		visualiser.load()->setup("NN", 0, layerSizes);
		visualiser.load()->mainLoop();
		});
}



void NeuralNetwork::train(DataSet& data, int epochs, bool debugTimes) {
	progress.trackNew("epoch", 0, epochs, true);
	cout << "Training Started" << "\n";
	epochElapsedTimesMS.clear();
	if (debugTimes) {
		stopwatch.start();
	}
	if (visualise) {
		threadVisualise();
		while (visualiser.load()->isSettingUp());
	}
	int inputSize = data.getInputs().size();
	int modulo = inputSize;
	if ((inputSize / 3) > 0) { modulo = inputSize / 3; }
	for (int e = 0; e < epochs; e++) {
		progress.trackNew("input", 0, inputSize, true);
		for (int i = 0; i < inputSize; ++i) {

			feedforward(data.getInputs()[i], e == 0 && i == 0, i % modulo == 0);
			backpropagate(data.getOutputs()[i]);
			progress.update("input", 1);
		}
		if (debugTimes) { epochElapsedTimesMS.push_back(stopwatch.elapsedMilliSeconds()); }
		progress.update("epoch", 1);
		//go in main loop
		cout << "\33[2K\r";
		cout << "Epoch: " << progress.getPercent("epoch") << " %"
			//<< "% | Input: " << progress.getPercent("input") << " %"
			<< flush;
	}
	cout << "\33[2K\r" << flush;
	if (debugTimes) { outputEpochTimes(); }
	threader.setSideThreadRunning(false);
}

void NeuralNetwork::test(DataSet& data, bool debugTimes, bool debugPredictions, float range) {
	int correctAmount = 0;
	auto& inputs = data.getInputs();
	auto& outputs = data.getOutputs();
	if (debugTimes) { stopwatch.start(); }
	for (size_t i = 0; i < inputs.size(); ++i) {
		feedforward(inputs[i]);
		auto predicted = getOutput();
		bool match = true;
		for (size_t j = 0; j < predicted.size(); ++j) {
			bool correct = abs(predicted[j] - outputs[i][j]) < range;
			if (debugPredictions) { outputPredictions(predicted, outputs[i], range); }
			if (!correct) {
				match = false;
				break;
			}
		}
		if (match) correctAmount++;
	}
	if (debugTimes) { outputTestTime(); }
	float accuracy = 100.0 * correctAmount / inputs.size();
	cout << "Accuracy: " << accuracy << "%" << "\n";
}

void NeuralNetwork::shutdown() {
	threader.setSideThreadRunning(false);            // Signal threads to stop
	threader.getCV().notify_all();                    // Wake all waiting threads

	if (threader.getMain().joinable())                // Join the worker thread
		threader.getMain().join();
	if (threader.getSide().joinable())                // Join the worker thread
		threader.getSide().join();



}


