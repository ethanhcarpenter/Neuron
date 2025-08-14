#include "Run.h";


//	vector<int> architecture;
//	bool visualiseNN;
//	int epochs;
//	float learningRate;
//	float range;
//	bool debugTimes;
//	bool debugPredictions;


int main(){
	RunParams runParameters = {
		{10,20,20,1},
		true,
		1000,
		0.1,
		0.05,
		false,
		false,
	};
	Run run(runParameters);
	auto rule = RuleBook::sumGreaterThanN(6);

	run.runRandomData(rule, 0.01);
}