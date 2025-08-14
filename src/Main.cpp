#include "Run.h";


//	vector<int> architecture;
//	bool visualiseNN;
//	int epochs;
//	float learningRate;
//	float range;
//  string activationType;



int main() {
	RunParams runParameters = {
		{10,256,128,1},
		true,
		100,
		1e-3,
		0.05,
		"relu",
	};
	Run run(runParameters);
	auto rule = RuleBook::sumGreaterThanN(6);

	run.runRandomData(rule,0.05);
}