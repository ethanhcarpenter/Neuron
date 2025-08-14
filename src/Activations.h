#pragma once

#include <string>

using namespace std;

class Activations {
public:
	static float activate(string type, float value);
	static float derive(string type, float value);
	static float sigmoid(float value);
	static float sigmoidDerivative(float value);
};


