#include "Progress.h"

void Progress::trackNew(string name, int initialAmount, int total, bool direction) {

	for (auto& map : tracking) {
		if (map.first == name) {
			refresh(name, initialAmount, total, direction);
			break;
		}
	}
	tracking.insert(make_pair(name, make_tuple(initialAmount, total, direction)));
}
void Progress::update(string name, int increment) {
	bool direction = std::get<Direction>(tracking[name]);
	get<CurrentAmount>(tracking[name]) += direction ? increment : increment * -1;
}
int Progress::getCurrent(string name) { return get<CurrentAmount>(tracking[name]); }
int Progress::getTotal(string name) { return get<TotalAmount>(tracking[name]); }
float Progress::getPercent(string name) {
	int current = get<CurrentAmount>(tracking[name]);
	int total = get<TotalAmount>(tracking[name]);
	float percent = static_cast<float>(current) / static_cast<float>(total);
	return percent*100;
}
void Progress::refresh(string name, int initialAmount, int total, bool direction) { tracking.erase(name); }