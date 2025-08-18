#include "Rules.h"


std::function<std::vector<float>(const std::vector<float>&)> RuleBook::sumGreaterThanN(float n) {
	return [n](const std::vector<float>& in) -> std::vector<float> {
		float sum = std::accumulate(in.begin(), in.end(), 0.0f);
		float result = (sum > n) ? 1.0f : 0.0f;
		return { result };
		};
}


