#pragma once
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cmath>
#include <thread>
#include <atomic>
#include <shared_mutex>
#include "Statistics.h"


constexpr double M_PI = 3.14159265358979323846;
using namespace std;

class Connection {
private:
	float fromX, fromY, toX, toY;
	bool drawn = true;
	float weight;
public:
	Connection(float fX, float fY, float tX, float tY, float w);
	void setDrawn(bool drew);
	const bool isDrawn();
	void setWeight(float w);
	const float getWeight();
	const float getFromX();
	const float getFromY();
	const float getToX();
	const float getToY();

};

class Visualiser {
private:
	atomic<shared_ptr<Statistics>*> stats;
	GLFWmonitor* targetMonitor;
	const GLFWvidmode* mode;
	GLFWwindow* window;
	vector<vector<pair<float, float>>> positions;
	vector<int> layers;
	pair<int, int> windowDimensions;
	vector<Connection> connections;
	unordered_map<string, int>connectionsIndexes;
	float lastUpdate;
	size_t currentConnection;
	bool startingAnimation;
	bool isSetup;
	float neuronRadius;
	ImGuiWindowFlags windowFlags;
public:
	Visualiser();
	void setup(const char* name, int targetMonitorIndex, vector<int> layerSizes,shared_ptr<Statistics>* s, int windowWidth = -1, int windowHeight = -1);
	const float getTabContentHeight();
	void generateNeuronPositions(const vector<int>& layers, float width, float height);
	void drawCircle(float cx, float cy, float r, int num_segments);
	const float calculateNeuronRadius(float height, float margin);
	void drawNeurons();
	tuple<float, float, float, float> generateColour(float weight);
	void drawConnections();
	void terminate();
	const int getCurrentConnection();
	vector<Connection>& getConnections();
	GLFWwindow* getWindow();
	void addConnectionIndex(int fromLayer, int from, int to);
	string generateConnectionUID(int fromLayer, int from, int to);
	const float getConnectionWeight(int fromLayer, int from, int to, float weight);
	void mainLoop(shared_ptr<shared_mutex>* stastMutex);
	const bool isSettingUp();
	void updateConnection(int fromLayer, int from, int to, float weight);


};