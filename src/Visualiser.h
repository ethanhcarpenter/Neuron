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
#define M_PI 3.14159265358979323846
using namespace std;

class Connection {
private:
	float fromX, fromY, toX, toY;
	bool drawn = true;
	float weight;
public:
	Connection(float fX, float fY, float tX, float tY, float w);
	void setDrawn(bool drew);
	bool isDrawn();
	void setWeight(float w);
	float getWeight();
	float getFromX();
	float getFromY();
	float getToX();
	float getToY();

};

class Visualiser {
private:
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
	void setup(const char* name, int targetMonitorIndex, vector<int> layerSizes, int windowWidth = -1, int windowHeight = -1);
	float getTabContentHeight();
	void generateNeuronPositions(const vector<int>& layers, float width, float height);
	void drawCircle(float cx, float cy, float r, int num_segments);
	float calculateNeuronRadius(float height, float margin);
	void drawNeurons();
	tuple<float, float, float, float> generateColour(float weight);
	void drawConnections();
	void terminate();
	int getCurrentConnection();
	vector<Connection>& getConnections();
	GLFWwindow* getWindow();
	void addConnectionIndex(int fromLayer, int from, int to);
	void addConnection(int fromLayer, int from, int to, float weight);
	void changeWeight(int fromLayer, int from, int to, float weight);
	string generateConnectionUID(int fromLayer, int from, int to, float weight);
	float getConnectionWeight(int fromLayer, int from, int to, float weight);
	void mainLoop();
	bool isSettingUp();
	void removeDuplicateConnections();
	void updateConnection(int fromLayer, int from, int to, float weight);


};