#include "NN.h"

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


NeuralNetwork::NeuralNetwork() {}

void NeuralNetwork::setup(bool v, vector<int> ls, float lr, string at) {
	activationType = at;
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

	statsMutex.store(new std::shared_ptr<std::shared_mutex>(std::make_shared<std::shared_mutex>()));
	visualiser.store(new std::shared_ptr<Visualiser>(std::make_shared<Visualiser>()));
	stats.store(new std::shared_ptr<Statistics>(std::make_shared<Statistics>(lr)));
	

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
				curr[j].setValue(Activations::activate(activationType, sum));
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
			curr[j].setValue(Activations::activate(activationType, sum));
		}
	}

	if (!updateWeights) { return; }
	unique_lock<mutex> lock(threader.getQueueMutex());
	threader.getQueue().emplace([updates = move(updates), firstPass, this, visualiser = visualiser.load()]() {
		if (!visualiser) return;
		int update = 0;
		for (auto& u : updates) {
			int layer = u.layer;
			int from = u.from;
			int to = u.to;
			float weight = u.weight;
			float lastweight = 1.1;
			if (!firstPass) lastweight = visualiser->get()->getConnectionWeight(layer, from, to, weight);
			if (roundf(abs(lastweight) * 10) == roundf(abs(weight) * 10)) continue;
			visualiser->get()->updateConnection(layer, from, to, weight);
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
		deltas.back()[i] = error * Activations::derive(activationType, outVal);
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

			deltas[l][i] = sum * Activations::derive(activationType, curr[i].getValue());
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


void NeuralNetwork::threadVisualise() {
	threader.getMain() = std::thread([&, this] {
		visualiser.load()->get()->setup("NN", 0, layerSizes,stats);
		visualiser.load()->get()->mainLoop(statsMutex);
		});
}

void NeuralNetwork::train(DataSet& data, int epochs) {
	if (auto mtxPtr = statsMutex.load()) {
		std::unique_lock<std::shared_mutex> lock(*mtxPtr->get());
		stats.load()->get()->setTotalInputs(data.getInputs().size());
	}

	stopwatch.start();

	if (visualise) {
		threadVisualise();
		while (visualiser.load()->get()->isSettingUp());
	}

	int inputSize = data.getInputs().size();
	int modulo = (inputSize / 3) > 0 ? inputSize / 3 : inputSize;

	for (int e = 0; e < epochs; e++) {
		if (auto mtxPtr = statsMutex.load()->get()) {
			std::unique_lock<std::shared_mutex> lock(*mtxPtr);
			stats.load()->get()->nextEpoch(stopwatch);
			stats.load()->get()->resetInput();
		}
		for (int i = 0; i < inputSize; ++i) {

			if (auto mtxPtr = statsMutex.load()->get()) {
				std::unique_lock<std::shared_mutex> lock(*mtxPtr);
				stats.load()->get()->nextInput();
			}

			feedforward(data.getInputs()[i], e == 0 && i == 0, i % modulo == 0);
			backpropagate(data.getOutputs()[i]);
		}
	}
	threader.setSideThreadRunning(true);
}

void NeuralNetwork::test(DataSet& data, float range) {
	int correctAmount = 0;
	auto& inputs = data.getInputs();
	auto& outputs = data.getOutputs();
	for (size_t i = 0; i < inputs.size(); ++i) {
		feedforward(inputs[i]);
		auto predicted = getOutput();
		bool match = true;
		for (size_t j = 0; j < predicted.size(); ++j) {
			bool correct = abs(predicted[j] - outputs[i][j]) < range;
			if (!correct) {
				match = false;
				break;
			}
		}
		if (match) correctAmount++;
	}
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


