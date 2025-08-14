#include "Activations.h"

float Activations::activate(string type,float value) {
	if (type == "sigmoid") { return sigmoid(value); }
	
}

float Activations::derive(string type, float value) {
	if (type == "sigmoid") { return sigmoidDerivative(value); }

}

float Activations::sigmoid(float value) {
	return 1.0 / (1.0 + exp(-value));
}

float Activations::sigmoidDerivative(float value) {
	return value * (1 - value);
}
