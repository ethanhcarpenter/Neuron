#include "Visualiser.h"
#include <iostream>

Connection::Connection(float fX, float fY, float tX, float tY, float w) {
	fromX = fX;
	fromY = fY;
	toX = tX;
	toY = tY;
	weight = w;
}
void Connection::setDrawn(bool drew) { drawn = drew; }
bool Connection::isDrawn() { return drawn; }
float Connection::getFromX() { return fromX; }
float Connection::getFromY() { return fromY; }
float Connection::getToX() { return toX; }
float Connection::getToY() { return toY; }
void Connection::setWeight(float w) { weight = w; }
float Connection::getWeight() { return weight; }



Visualiser::Visualiser() :isSetup(false) {};
void Visualiser::setup(const char* name, int targetMonitorIndex, vector<int> layerSizes, int windowWidth, int windowHeight) {
	if (!glfwInit()) return;
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	targetMonitor = monitors[targetMonitorIndex];
	mode = glfwGetVideoMode(targetMonitor);
	windowDimensions.first = (windowWidth == -1) ? mode->width : windowWidth;
	windowDimensions.second = (windowHeight == -1) ? mode->height : windowHeight;
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(
		windowDimensions.first,
		windowDimensions.second,
		name,
		NULL,  
		NULL
	);

	if (!window) {
		glfwTerminate();
	}
	int xpos, ypos;
	glfwGetMonitorPos(targetMonitor, &xpos, &ypos);
	glfwSetWindowPos(window, xpos, ypos);
	glfwMakeContextCurrent(window);

	IMGUI_CHECKVERSION();
	windowFlags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse;
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");


	layers = layerSizes;
	lastUpdate = glfwGetTime();
	currentConnection = 0;
	startingAnimation = true;
	float usableHeight = getTabContentHeight();
	generateNeuronPositions(layers, windowDimensions.first, usableHeight);
	neuronRadius = calculateNeuronRadius(usableHeight, 1.0f);
	isSetup = true;
}

float Visualiser::getTabContentHeight() {
	float usableHeight = 0;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Begin your tab bar and measure available height
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin("Dummy", nullptr, windowFlags);

	if (ImGui::BeginTabBar("Tabs")) {
		ImGui::BeginTabItem("Neural Network");
		usableHeight = ImGui::GetContentRegionAvail().y;
		ImGui::EndTabItem();
		ImGui::EndTabBar();
		;
	}

	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	return usableHeight;
}

void Visualiser::generateNeuronPositions(const vector<int>& layers, float width, float height) {
	if (layers.empty()) { return; }
	float layerSpacingX = width / (layers.size() + 1);
	for (size_t i = 0; i < layers.size(); i++) {
		vector<pair<float, float>> layerPositions;
		float neuronSpacingY = height / (layers[i] + 1);

		for (int j = 0; j < layers[i]; j++) {
			float x = (i + 1) * layerSpacingX;
			float y = (j + 1) * neuronSpacingY;
			y += (windowDimensions.second - height);
			layerPositions.push_back({ x, y });
		}
		positions.push_back(layerPositions);
	}
	int a = 12;
}

void Visualiser::drawCircle(float cx, float cy, float r, int num_segments) {
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(cx, cy);
	for (int i = 0; i <= num_segments; i++) {
		float theta = 2.0f * M_PI * float(i) / float(num_segments);
		float x = r * cosf(theta);
		float y = r * sinf(theta);
		glVertex2f(cx + x, cy + y);
	}
	glEnd();
}

float Visualiser::calculateNeuronRadius(float height, float margin) {
	int maxNeurons = *max_element(layers.begin(), layers.end());
	float spacingY = height / (maxNeurons + 1);
	return (spacingY / 2.0f) - margin;
}

void Visualiser::drawNeurons() {
	glColor3f(0.0f, 0.0f, 1.0f);
	for (auto& layer : positions) {
		int a = 12;
		for (auto& neuron : layer) {
			drawCircle(neuron.first, neuron.second, neuronRadius, 64);
		}
	}
}

int Visualiser::getCurrentConnection() { return currentConnection; }
vector<Connection>& Visualiser::getConnections() { return connections; }

tuple<float, float, float, float> Visualiser::generateColour(float weight) {
	float opacity = min(1.0f, abs(weight));
	if (weight < 0) {
		return { 1.0f, 0.0f, 0.0f, opacity };
	}
	else {
		return { 0.0f, 1.0f, 0.0f, opacity };
	}
}

void Visualiser::drawConnections() {
	for (auto& c : connections) {
		if (!c.isDrawn()) continue;
		auto colour = generateColour(c.getWeight());
		float lineWidth = roundf( abs(c.getWeight()) *10.0f);
		glLineWidth(lineWidth);
		glBegin(GL_LINES);
		glColor4f(get<0>(colour), get<1>(colour), get<2>(colour), 1.0f);
		glVertex2f(c.getFromX(), c.getFromY());
		glVertex2f(c.getToX(), c.getToY());
		glEnd();
	}
	glLineWidth(1.0f);
}

bool Visualiser::isSettingUp() { return !isSetup; }
string Visualiser::generateConnectionUID(int fromLayer, int from, int to, float weight) {
	string uid = to_string(fromLayer) + to_string(from) + to_string(to);
	return uid;
 }

float Visualiser::getConnectionWeight(int fromLayer, int from, int to, float weight) {
	string uid = generateConnectionUID(fromLayer, from, to, weight);
	auto it = connectionsIndexes.find(uid);
	int index = it->second;
	return connections.at(index).getWeight();
}

void Visualiser::updateConnection(int fromLayer, int from, int to, float weight) {
	
	string uid = generateConnectionUID(fromLayer, from, to, weight);
	auto it = connectionsIndexes.find(uid);
	if (it == connectionsIndexes.end()) {
		// New connection: add with positions and weight
		float fromX = positions[fromLayer][from].first;
		float fromY = positions[fromLayer][from].second;
		float toX = positions[fromLayer + 1][to].first;
		float toY = positions[fromLayer + 1][to].second;

		connections.push_back({ fromX, fromY, toX, toY, weight });
		connectionsIndexes[uid] = static_cast<int>(connections.size() - 1);
	}
	else {
		// Existing connection: update weight
		int index = it->second;
		//cout << "updated from: " << connections.at(index).getWeight() << " to: " << weight << "\n";
		connections.at(index).setWeight(weight);
	}
}
GLFWwindow* Visualiser::getWindow() { return window; }
void Visualiser::addConnectionIndex(int fromLayer, int from, int to) {
	string uid = "";
	uid += to_string(fromLayer);
	uid += to_string(from);
	uid += to_string(to);
	connectionsIndexes[uid] = static_cast<int>(connections.size() - 1);
}
//void Visualiser::addConnection(int fromLayer, int from, int to, float weight) {
//	float fromX, fromY, toX, toY;
//	fromX = positions[fromLayer][from].first;
//	fromY = positions[fromLayer][from].second;
//	toX = positions[fromLayer + 1][to].first;
//	toY = positions[fromLayer + 1][to].second;
//	connections.push_back({ fromX,fromY,toX,toY,weight });
//	removeDuplicateConnections();
//}
//
//void Visualiser::removeDuplicateConnections() {
//	auto isSame = [](Connection& a, Connection& b) {
//		return a.getFromX() == b.getFromX() &&
//			a.getFromY() == b.getFromY() &&
//			a.getToX() == b.getToX() &&
//			a.getToY() == b.getToY();
//		};
//	sort(connections.begin(), connections.end(),
//		[](Connection& a, Connection& b) {
//			if (a.getFromX() != b.getFromX()) return a.getFromX() < b.getFromX();
//			if (a.getFromY() != b.getFromY()) return a.getFromY() < b.getFromY();
//			if (a.getToX() != b.getToX())   return a.getToX() < b.getToX();
//			return a.getToY() < b.getToY();
//		}
//	);
//	connections.erase(
//		unique(connections.begin(), connections.end(), isSame),
//		connections.end()
//	);
//}
//
//void Visualiser::changeWeight(int fromLayer, int from, int to, float weight) {
//	string uid = "";
//	uid += to_string(fromLayer);
//	uid += to_string(from);
//	uid += to_string(to);
//	int index = connectionsIndexes[uid];
//	if (index == 12) {
//		int a = 12;
//	}
//	connections.at(index).setWeight(weight);
//}


void Visualiser::mainLoop() {
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		});

	while (!glfwWindowShouldClose(window)) {
		// poll inputs
		glfwPollEvents();

		// clear + draw background
		int winWidth, winHeight;
		glfwGetFramebufferSize(window, &winWidth, &winHeight);
		glViewport(0, 0, winWidth, winHeight);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// draw UI
		bool showNN = false;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		if (ImGui::Begin("Main Tabs", nullptr, windowFlags)) {
			if (ImGui::BeginTabBar("Tabs")) {
				if (ImGui::BeginTabItem("Neural Network")) {
					showNN = true;
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Tab 2")) {
					ImGui::Text("Content for Tab 2");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Tab 3")) {
					ImGui::Text("Content for Tab 3");

					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();

		// render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// draw NN overlay
		if (showNN) {
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, winWidth, winHeight, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			drawNeurons();
			drawConnections();

			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		// swap buffers
		glfwSwapBuffers(window);
	}
	terminate();

}

void Visualiser::terminate() {
	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Cleanup GLFW
	glfwSetWindowShouldClose(window, true);
	glfwWindowShouldClose(window);
	glfwTerminate();
}



//no need
//if (startingAnimation) {
//
//	float now = glfwGetTime();
//	if (now - lastUpdate >= delayPerConnection && currentConnection < connections.size()) {
//		connections[currentConnection].setDrawn(true);
//		currentConnection++;
//		lastUpdate = now;
//	}
//	else if (currentConnection != 0 && currentConnection == connections.size()) { startingAnimation = false; }
//}



