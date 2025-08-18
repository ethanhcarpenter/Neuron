#include "Activations.h"



#pragma region Activate
float Activations::activate(std::string type, float x) {

	std::unordered_map<std::string, std::function<float(float)>> activations = {
		{"sigmoid", activateSigmoid},
		{"tanh", activateTanh},
		{"relu", activateRelu},
		{"leakyrelu", activateLeakyrelu},
		{"linear", activateLinear}
	};

	auto it = activations.find(type);
	return it != activations.end() ? it->second(x) : x;
}
float Activations::activateRelu(float x) {
	return std::max(0.0f, x);
}
float Activations::activateLeakyrelu(float x) {
	return x > 0 ? x : 0.01f * x;
}
float Activations::activateLinear(float x) {
	return x;
}
float Activations::activateSigmoid(float x) {
	x = std::max(std::min(x, 50.0f), -50.0f);
	return 1.0f / (1.0f + std::exp(-x));
}
float Activations::activateTanh(float x) {
	x = std::max(std::min(x, 20.0f), -20.0f);
	return std::tanh(x);
}
#pragma endregion



#pragma region Derive
float Activations::derive(std::string type, float x) {

	std::unordered_map<std::string, std::function<float(float)>> derivatives = {
		{"sigmoid", deriveSigmoid},
		{"tanh", deriveTanh},
		{"relu", deriveRelu},
		{"leakyrelu", deriveLeakyrelu},
		{"linear", deriveLinear}
	};

	auto it = derivatives.find(type);
	return it != derivatives.end() ? it->second(x) : x;
}
float Activations::deriveRelu(float x) {
	return x > 0.0f ? 1.0f : 0.0f;
}
float Activations::deriveLeakyrelu(float x) {
	return x > 0.0f ? 1.0f : 0.01f;
}
float Activations::deriveLinear(float x) {
	return 1.0f;
}
float Activations::deriveSigmoid(float x) {
	return x * (1.0f - x);
}
float Activations::deriveTanh(float x) {
	return 1.0f - x * x;
}
#pragma endregion