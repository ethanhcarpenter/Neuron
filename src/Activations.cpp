#include "Activations.h"

float Activations::activate(string type,float value) {
	if (type == "sigmoid") { return sigmoid(value); }
	else if (type == "relu") { return relu(value); }
	
}

float Activations::derive(string type, float value) {
	if (type == "sigmoid") { return sigmoidDerivative(value); }
	else if (type == "relu") { return reluDerivative(value); }

}

float Activations::sigmoid(float value) {
	return 1.0 / (1.0 + exp(-value));
}

float Activations::sigmoidDerivative(float value) {
	return value * (1 - value);
}

float Activations::relu(float value) {
	return std::max(0.0f, value); 
}

float Activations::reluDerivative(float value) {
	return value > 0.0f ? 1.0f : 0.0f;
}
