#include "Rules.h"

function<vector<float>(const vector<float>&)> RuleBook::sumGreaterThanN(float n) {
	return [n](const vector<float>& in) -> vector<float> {
		float sum = accumulate(in.begin(), in.end(), 0.0f);
		float result = (sum > n) ? 1.0f : 0.0f;
		return { result };
		};
}


