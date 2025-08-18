#pragma once
#include <queue>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>




class Threader {
private:
	std::queue<std::function<void()>> sideSideThreadFunction;
	std::atomic<bool> sideAlternateThreadRunning;
	std::mutex queueMutex;
	std::condition_variable cv;
	std::thread mainAlternateThread;
	std::thread sideAlternateThread;
public:
	Threader();
	void visualiserWorker();
	std::thread& getMain();
	std::thread& getSide();
	std::mutex& getQueueMutex();
	std::queue<std::function<void()>>& getQueue();
	std::condition_variable& getCV();
	void setSideThreadRunning(bool v);

};

