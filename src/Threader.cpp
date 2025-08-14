#include"Threader.h"

Threader::Threader() :sideAlternateThreadRunning(true) {};

void Threader::visualiserWorker() {
	while (true) {
		function<void()> task;
		{
			unique_lock<mutex> lock(queueMutex);
			cv.wait(lock, [&] { return !sideAlternateThreadRunning || !sideSideThreadFunction.empty(); });
			if (!sideAlternateThreadRunning && sideSideThreadFunction.empty())
				break;
			task = move(sideSideThreadFunction.front());
			sideSideThreadFunction.pop();
		}
		task();
	}
}

void Threader::setSideThreadRunning(bool v) { sideAlternateThreadRunning = v; }
thread& Threader::getMain() { return mainAlternateThread; }
thread& Threader::getSide() { return sideAlternateThread; }
mutex& Threader::getQueueMutex() { return queueMutex; }
queue<function<void()>>& Threader::getQueue() { return sideSideThreadFunction; }
condition_variable& Threader::getCV() { return cv; }
