#include "Run.h"


Run::Run(){
	nn.setup();
}
void Run::runNumbers(int quality,int trainAmount,int testAmount) {
	
	DataSet data = { static_cast<int>(pow(quality,2)) };
	std::ostringstream oss;
	oss << "data\\data" << quality << quality << ".txt";
	std::string filename = oss.str();

	data.generateImageDataFromTextFileRandom(filename.c_str(), trainAmount);
	nn.train(data);

	data.generateImageDataFromTextFileRandom(filename.c_str(), testAmount);
	nn.test(data);

	nn.shutdown();
}




//void Run::runRandomData(std::function<std::vector<float>(const std::vector<float>&)> rule, float noise) {
//
//	DataSet data = { nn.numberOfInputs() };
//
//	data.generateDataset(true, noise, rule);
//	nn.train(data);
//
//	data.generateDataset(false, noise, rule);
//	nn.test(data);
//
//	nn.shutdown();
//}