#pragma once
#include <vector>
#include <functional>
#include <fstream>
#include <chrono>
#include <string>
#include <random>


class DataSet {
private:
	std::vector<std::vector<float>> inputs;
	std::vector<std::vector<float>> outputs;
	int numberOfInputs;
public:
	DataSet(int inputNumber);
	void generateDataset(bool train, float noise, std::function<std::vector<float>(const std::vector<float>&)> rule);
	void generateImageDataFromTextFile(const char* path, int amount);
	void generateImageDataFromTextFileRandom(const char* path, int amount);
	std::vector<std::vector<float>>& getInputs();
	std::vector<std::vector<float>>& getOutputs();
};