#pragma once
#include <string>
#include <unordered_map>
#include <functional>


class Activations {
public:

	static float activate(std::string type, float x);
	static float activateSigmoid(float x);
	static float activateRelu(float x);
	static float activateLeakyrelu(float x);
	static float activateTanh(float x);
	static float activateLinear(float x);

	static float derive(std::string type, float x);
	static float deriveSigmoid(float x);
	static float deriveRelu(float x);
	static float deriveLeakyrelu(float x);
	static float deriveTanh(float x);
	static float deriveLinear(float x);
};


