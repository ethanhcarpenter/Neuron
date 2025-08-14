#pragma once
#include <unordered_map>
#include <string>
#include <tuple>

using namespace std;

enum TupleIndexes {
	CurrentAmount,
	TotalAmount,
	Direction,
};

class Progress {
private:
	unordered_map<string, tuple<int, int, bool>>tracking;
public:
	void trackNew(string name, int initialAmount, int total, bool direction);
	void update(string name, int increment);
	int getCurrent(string name);
	int getTotal(string name);
	float getPercent(string name);
	void refresh(string name, int initialAmount, int total, bool direction);
};