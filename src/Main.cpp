#include "Run.h";


//	vector<int> architecture;
//	bool visualiseNN;
//	int epochs;
//	float learningRate;
//	float range;
//  string activationType;



int main() {
	RunParams runParameters = {
		{7 * 7,20,20,1},
		true,
		1000,
		0.1,
		0.05,
		"sigmoid",
	};
	Run run(runParameters);
	auto rule = RuleBook::sumGreaterThanN(6);

	run.runNumbers(7);
}