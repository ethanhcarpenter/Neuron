#pragma once

#include <vector>
#include <functional>
#include <random>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

class DataSet {
private:
	vector<vector<float>> inputs;
	vector<vector<float>> outputs;
	int numberOfInputs;
public:
	DataSet(int inputNumber);
	void generateDataset(bool train,float noise, function<vector<float>(const vector<float>&)> rule);
	void generateImageDataFromTextFile(const char* path, int amount);
	void generateImageDataFromTextFileRandom(const char* path, int amount);
	vector<vector<float>>& getInputs();
	vector<vector<float>>& getOutputs();
};