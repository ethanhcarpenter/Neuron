#pragma once
#include <functional>
#include <numeric>


class RuleBook {
public:
	static std::function<std::vector<float>(const std::vector<float>&)> sumGreaterThanN(float n);
};

