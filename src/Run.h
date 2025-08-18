#pragma once
#include <functional>
#include <sstream>
#include <string>

#include "NeuralNetwork.h"
#include "DataSet.h"
#include "Rules.h"


class Run {
private:

	NeuralNetwork nn;
public:
	Run();
	void runNumbers(int quality, int trainAount, int testAmount);
	void runRandomData(std::function<std::vector<float>(const std::vector<float>&)> rule, float noise);
};