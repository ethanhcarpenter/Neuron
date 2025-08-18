#include "Threader.h"


#pragma region Initialise
Threader::Threader() :sideAlternateThreadRunning(true) {};
#pragma endregion



#pragma region Worker
void Threader::visualiserWorker() {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			cv.wait(lock, [&] { return !sideAlternateThreadRunning || !sideSideThreadFunction.empty(); });
			if (!sideAlternateThreadRunning && sideSideThreadFunction.empty()) { break; }
			task = std::move(sideSideThreadFunction.front());
			sideSideThreadFunction.pop();
		}
		task();
	}
}
#pragma endregion



#pragma region Get
std::thread& Threader::getMain() { return mainAlternateThread; }
std::thread& Threader::getSide() { return sideAlternateThread; }
std::mutex& Threader::getQueueMutex() { return queueMutex; }
std::queue<std::function<void()>>& Threader::getQueue() { return sideSideThreadFunction; }
std::condition_variable& Threader::getCV() { return cv; }
#pragma endregion



#pragma region Set
void Threader::setSideThreadRunning(bool v) { sideAlternateThreadRunning = v; }
#pragma endregion
