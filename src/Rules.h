#pragma once
#include <vector>
#include <numeric>
#include <cmath>
#include <functional>
#include <algorithm>

using namespace std;

class RuleBook {
public:
	static function<vector<float>(const vector<float>&)> sumGreaterThanN(float n);
};

