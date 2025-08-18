#include "DataSet.h"



DataSet::DataSet(int inputNumber) :numberOfInputs(inputNumber) {}

void DataSet::generateImageDataFromTextFile(const char* path, int amount) {
	srand(std::time(0));
	inputs.clear();
	outputs.clear();
	std::ifstream file(path);
	std::string line;
	int amountAdded = 0;
	while (amountAdded < amount && getline(file, line)) {
		int label;
		std::vector<float> pixels;
		float val;
		std::istringstream iss(line);
		iss >> label;
		while (iss >> val) {
			pixels.push_back(val);
		}
		inputs.push_back(pixels);
		std::vector<float> target(10, 0.0f);
		target[label] = 1.0f;
		outputs.push_back(target);
		amountAdded++;
	}
	file.close();
}
void DataSet::generateImageDataFromTextFileRandom(const char* path, int amount) {
	srand(time(0));
	inputs.clear();
	outputs.clear();
	std::ifstream file(path, std::ios::binary);
	std::vector<std::streampos> offsets;
	offsets.push_back(file.tellg());
	std::string line;
	while (std::getline(file, line)) {
		offsets.push_back(file.tellg());
	}
	if (amount > (int)offsets.size() - 1) amount = offsets.size() - 1;
	std::vector<int> indices(offsets.size() - 1);
	for (int i = 0; i < (int)indices.size(); ++i) indices[i] = i;
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(indices.begin(), indices.end(), g);
	indices.resize(amount);
	for (int idx : indices) {
		file.clear();
		file.seekg(offsets[idx]);

		if (std::getline(file, line)) {
			int label;
			std::vector<float> pixels;
			std::istringstream iss(line);
			iss >> label;
			float val;
			while (iss >> val) pixels.push_back(val);

			inputs.push_back(pixels);
			std::vector<float> target(10, 0.0f);
			target[label] = 1.0f;
			outputs.push_back(target);

		}
	}
	file.close();
}

void DataSet::generateDataset(bool train,float noise, std::function<std::vector<float>(const std::vector<float>&)> rule) {
	srand(time(0));
	std::random_device rd;
	std::mt19937 g(rd());
	inputs.clear();
	outputs.clear();
	int total = 1 << numberOfInputs;
	int sampleSize = total / (train ? 8 : 10);
	std::vector<std::vector<float>> allInputs;
	for (int i = 0; i < total; ++i) {
		std::vector<float> in(numberOfInputs);
		for (int bit = 0; bit < numberOfInputs; ++bit) {
			float base = ((i >> bit) & 1);
			if (noise > 0)
				base += ((rand() % 100) / 100.0 * 2 - 1) * noise;
			in[bit] = base;
		}
		allInputs.push_back(in);
	}
	shuffle(allInputs.begin(), allInputs.end(), g);

	for (int i = 0; i < sampleSize; ++i) {
		inputs.push_back(allInputs[i]);
		outputs.push_back(rule(allInputs[i]));
	}
}


std::vector<std::vector<float>>& DataSet::getInputs() {
	return inputs;
}

std::vector<std::vector<float>>& DataSet::getOutputs() {
	return outputs;
}