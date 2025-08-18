#include "NeuralNetwork.h"


static float randomWeight(const std::string& activationType, int fanIn, int fanOut) {
	static std::mt19937 gen(std::random_device{}());

	if (activationType == "relu" || activationType == "leakyrelu") {
		std::normal_distribution<float> dist(0.0f, std::sqrt(2.0f / fanIn));
		return dist(gen);
	}
	else if (activationType == "sigmoid" || activationType == "tanh") {
		float limit = std::sqrt(6.0f / (fanIn + fanOut));
		std::uniform_real_distribution<float> dist(-limit, limit);
		return dist(gen);
	}
	else {
		std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
		return dist(gen);
	}
}

#pragma region Node-Class

#pragma region Initialise
Node::Node(NodeType t) : type(t), value(0) {}
#pragma endregion


#pragma region Get
float Node::getValue() { return value; }
float Node::getBias() { return bias; }
NodeType Node::getType() { return type; }
#pragma endregion


#pragma region Set
void Node::setValue(float v) { value = v; }
void Node::setBias(float b) { bias = b; }
#pragma endregion

#pragma endregion



#pragma region Layer-Class

#pragma region Initialise
Layer::Layer(int size, NodeType type) {
	for (int i = 0; i < size; ++i)
		nodes.emplace_back(type);
}
#pragma endregion


#pragma region Get
std::vector<Node>& Layer::getNodes() { return nodes; }
int Layer::getSize() { return nodes.size(); }
#pragma endregion

#pragma endregion



#pragma region NeuralNetwork-Setup
NeuralNetwork::NeuralNetwork() {}
void NeuralNetwork::setup() {
	statsMutex.store(new std::shared_ptr<std::shared_mutex>(std::make_shared<std::shared_mutex>()));
	visualiser.store(new std::shared_ptr<Visualiser>(std::make_shared<Visualiser>()));
	stats.store(new std::shared_ptr<Statistics>(std::make_shared<Statistics>()));
	maxEpochs = 0;
	threadVisualise();

	while (!(visualiser.load()->get()->isSettingUp()) == 0);
	{
		std::shared_lock<std::shared_mutex> lock(*statsMutex.load()->get());
		activationType = stats.load()->get()->getActivationType();
		learningRate = stats.load()->get()->getLearningRate();
		layerSizes = stats.load()->get()->getLayerSizes();
	}
	int index = 0;
	for (int size : layerSizes) {
		NodeType type = (index == 0 ? NodeType::Input :
			(index == layerSizes.size() - 1 ? NodeType::Output : NodeType::Hidden));
		layers.emplace_back(size, type);
		index++;
	}
	initWeights();
	threader.getSide() = std::thread(&Threader::visualiserWorker, &threader);
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
				weights[l][i][j] = randomWeight(activationType, fromSize, toSize);
			}
		}
		for (auto& node : layers[l + 1].getNodes()) {
			node.setBias(randomWeight(activationType, fromSize, toSize));
		}
	}
}
void NeuralNetwork::shutdown() {
	threader.setSideThreadRunning(false);            // Signal threads to stop
	threader.getCV().notify_all();                    // Wake all waiting threads

	if (threader.getMain().joinable())                // Join the worker std::thread
		threader.getMain().join();
	if (threader.getSide().joinable())                // Join the worker std::thread
		threader.getSide().join();



}
#pragma endregion



#pragma region Run-Through
void NeuralNetwork::feedforward(std::vector<float>& inputVals, bool firstPass, bool updateWeights) {
	auto& inputLayer = layers[0].getNodes();
	for (int i = 0; i < inputVals.size(); ++i) { inputLayer[i].setValue(inputVals[i]); }


	std::vector<VisualUpdate> updates;

	for (size_t l = 1; l < layers.size(); ++l) {
		auto& prev = layers[l - 1].getNodes();
		auto& curr = layers[l].getNodes();
		std::vector<float> sums(curr.size());


		for (size_t j = 0; j < curr.size(); ++j) {
			float sum = curr[j].getBias();
			for (size_t i = 0; i < prev.size(); ++i) {
				sum += prev[i].getValue() * weights[l - 1][i][j];
				updates.push_back({ int(l - 1),int(i),int(j),weights[l - 1][i][j] });
			}
			sums[j] = sum;
		}

		for (size_t j = 0; j < curr.size(); ++j) {
			sums[j] = Activations::activate(activationType, sums[j]);
		}

		for (size_t j = 0; j < curr.size(); ++j)
			curr[j].setValue(sums[j]);
	}

	if (!updateWeights) { return; }

	std::unique_lock<std::mutex> lock(threader.getQueueMutex());
	if (threader.getQueue().size() != 0) { return; }
	threader.getQueue().emplace([updates = std::move(updates), firstPass, this, visualiser = visualiser.load()]() {
		if (!visualiser) return;
		int update = 0;
		for (auto& u : updates) {
			int layer = u.layer;
			int from = u.from;
			int to = u.to;
			float weight = u.weight;
			float lastweight = 1.1f;
			if (!firstPass) { lastweight = visualiser->get()->getConnectionWeight(layer, from, to, weight); }
			visualiser->get()->updateConnection(layer, from, to, weight);
		}
		});

	threader.getCV().notify_one();
}
void NeuralNetwork::backpropagate(std::vector<float>& targetVals) {
	std::vector<std::vector<float>> deltas(layers.size());

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
			float sum = 0.0f;
			for (int j = 0; j < nextSize; ++j)
				sum += weights[l][i][j] * deltas[l + 1][j];
			deltas[l][i] = sum * Activations::derive(activationType, curr[i].getValue());
		}
	}

	for (size_t l = 0; l < weights.size(); ++l) {
		auto& from = layers[l].getNodes();
		auto& to = layers[l + 1].getNodes();
		const int fromSize = from.size();
		const int toSize = to.size();

		for (int i = 0; i < fromSize; ++i) {
			for (int j = 0; j < toSize; ++j) {
				weights[l][i][j] += learningRate * deltas[l + 1][j] * from[i].getValue();
			}

		}


		for (int j = 0; j < toSize; ++j) {
			to[j].setBias(to[j].getBias() + learningRate * deltas[l + 1][j]);
		}
	}
}
#pragma endregion



#pragma region Visualise
void NeuralNetwork::threadVisualise() {
	threader.getMain() = std::thread([&, this] {
		visualiser.load()->get()->setup("Neural Network", 0, stats);
		visualiser.load()->get()->mainLoop(statsMutex);
		});
}
void NeuralNetwork::updateParametersFromVisualiser() {
	auto& s = *stats.load()->get();
	activationType = s.getActivationType();
	learningRate = s.getLearningRate();
	maxEpochs = s.getMaxEpochs();
}
#pragma endregion



#pragma region Train
void NeuralNetwork::train(DataSet& data) {
	if (auto mtxPtr = statsMutex.load()) {
		std::unique_lock<std::shared_mutex> lock(*mtxPtr->get());
		maxEpochs = stats.load()->get()->getMaxEpochs();
		stats.load()->get()->setTotalInputs(data.getInputs().size());
	}
	stopwatch.start();

	int inputSize = data.getInputs().size();
	for (int epoch = 0; epoch < maxEpochs; ++epoch) {
		for (int i = 0; i < inputSize; ++i) {
			if (auto mtxPtr = statsMutex.load()) {
				std::shared_lock<std::shared_mutex> lock(*statsMutex.load()->get());
				if (stats.load()->get()->getNeuralNetworkNeedsUpdating()) {
					updateParametersFromVisualiser();
					stats.load()->get()->setNeuralNetworkNeedsUpdating(false);
				}
				if (!stats.load()->get()->getRunning()) { i--;  continue; }
			}

			auto& inputVals = data.getInputs()[i];
			auto& targetVals = data.getOutputs()[i];

			feedforward(inputVals, epoch == 0, true);
			backpropagate(targetVals);

			if (auto mtxPtr = statsMutex.load()) {
				std::unique_lock<std::shared_mutex> lock(*mtxPtr->get());
				stats.load()->get()->nextInput();
				stats.load()->get()->setWeights(weights);
			}
		}
		if (auto mtxPtr = statsMutex.load()) {
			std::unique_lock<std::shared_mutex> lock(*mtxPtr->get());
			stats.load()->get()->nextEpoch(stopwatch);
			stats.load()->get()->resetInput();
		}
	}

}
#pragma endregion



#pragma region Test
void NeuralNetwork::test(DataSet& data) {
	int correctAmount = 0;
	auto& inputs = data.getInputs();
	auto& outputs = data.getOutputs();

	for (size_t i = 0; i < inputs.size(); ++i) {
		feedforward(inputs[i], false, false);
		auto predicted = getPredictedOutput();

		size_t predictedClass = std::distance(predicted.begin(), std::max_element(predicted.begin(), predicted.end()));
		size_t actualClass = std::distance(outputs[i].begin(), std::max_element(outputs[i].begin(), outputs[i].end()));

		if (predictedClass == actualClass) {
			correctAmount++;
		}
	}

	float accuracy = 100.0f * correctAmount / inputs.size();
	if (auto mtxPtr = statsMutex.load()) {
		std::unique_lock<std::shared_mutex> lock(*mtxPtr->get());
		stats.load()->get()->setTestAccuracy(accuracy);
	}
}
std::vector<float> NeuralNetwork::getPredictedOutput() {
	std::vector<float> output;
	for (auto& node : layers.back().getNodes()) {
		output.push_back(node.getValue());
	}

	return output;
}
#pragma endregion







