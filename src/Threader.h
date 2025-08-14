#pragma once
#include <thread>
#include <atomic>
#include <queue>
#include <functional>
#include <iostream>
#include <mutex>
using namespace std;

class Threader {
private:
	queue<function<void()>> sideSideThreadFunction;
	atomic<bool> sideAlternateThreadRunning;
	mutex queueMutex;
	condition_variable cv;
	thread mainAlternateThread;
	thread sideAlternateThread;
public:
	Threader();
	void visualiserWorker();
	thread& getMain();
	thread& getSide();
	mutex& getQueueMutex();
	queue<function<void()>>& getQueue();
	condition_variable& getCV();
	void setSideThreadRunning(bool v);

};

