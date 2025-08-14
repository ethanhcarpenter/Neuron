#include "Visualiser.h"
#include <iostream>


Connection::Connection(float fX, float fY, float tX, float tY, float w) {
	fromX = fX;
	fromY = fY;
	toX = tX;
	toY = tY;
	weight = w;
}
const float Connection::getFromX() { return fromX; }
const float Connection::getFromY() { return fromY; }
const float Connection::getToX() { return toX; }
const float Connection::getToY() { return toY; }
void Connection::setWeight(float w) { weight = w; }
const float Connection::getWeight() { return weight; }



Visualiser::Visualiser() :isSetup(false) {};
void Visualiser::setup(const char* name, int targetMonitorIndex, vector<int> layerSizes, shared_ptr<Statistics>* s, int windowWidth, int windowHeight) {
	if (!glfwInit()) return;
	int count;
	stats.store(s);
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
	windowFlags = ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoBringToFrontOnFocus;

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
	calculatedConnectionCount = 0;
	for (int i = 0; i < layers.size() - 1; i++) {
		int product = layers[i] * layers[i + 1];
		calculatedConnectionCount += product;
	}
}

const float Visualiser::getTabContentHeight() {
	float usableHeight = 0;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

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

const float Visualiser::calculateNeuronRadius(float height, float margin) {
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

const int Visualiser::getCurrentConnection() { return currentConnection; }
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
	
	if (connections.size() != calculatedConnectionCount) { return; }
	for (auto& c : connections) {
		auto colour = generateColour(c.getWeight());
		float lineWidth = roundf(abs(c.getWeight()) * 10.0f);
		glLineWidth(lineWidth);
		glBegin(GL_LINES);
		glColor4f(get<0>(colour), get<1>(colour), get<2>(colour), 1.0f);
		glVertex2f(c.getFromX(), c.getFromY());
		glVertex2f(c.getToX(), c.getToY());
		glEnd();
	}
	glLineWidth(1.0f);
}

const bool Visualiser::isSettingUp() { return !isSetup; }
string Visualiser::generateConnectionUID(int fromLayer, int from, int to) {
	string uid = "from layer:" + to_string(fromLayer) + " node: " + to_string(from) + " To layer: " + to_string(fromLayer+1)+" node: " + to_string(to);
	return uid;
}

const float Visualiser::getConnectionWeight(int fromLayer, int from, int to, float weight) {
	string uid = generateConnectionUID(fromLayer, from, to);
	auto it = connectionsIndexes.find(uid);
	int index = it->second;
	return connections.at(index).getWeight();
}

void Visualiser::updateConnection(int fromLayer, int from, int to, float weight) {

	string uid = generateConnectionUID(fromLayer, from, to);
	auto it = connectionsIndexes.find(uid);
	if (it == connectionsIndexes.end()) {
		float fromX = positions[fromLayer][from].first;
		float fromY = positions[fromLayer][from].second;
		float toX = positions[fromLayer + 1][to].first;
		float toY = positions[fromLayer + 1][to].second;

		connections.push_back({ fromX, fromY, toX, toY, weight });
		connectionsIndexes[uid] = static_cast<int>(connections.size() - 1);
	}
	else {
		int index = it->second;
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

void Visualiser::mainLoop(shared_ptr<shared_mutex>* statsMutex) {
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		});

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		int winWidth, winHeight;
		glfwGetFramebufferSize(window, &winWidth, &winHeight);
		glViewport(0, 0, winWidth, winHeight);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Main tab window
		bool showNN = false;
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
			ImGui::End();
		}


		if (showNN) {
			//scope for the lock as to  not perma lock throughout the mainloop
			{
				std::shared_lock<std::shared_mutex> lock(*statsMutex->get());

				ImGui::SetNextWindowPos(ImVec2(winWidth - 320, 100), ImGuiCond_Always);
				ImGui::Begin("Training Overlay", nullptr, windowFlags);

				ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "Epoch %d", stats.load()->get()->getEpoch());

				float inputProgress = static_cast<float>(stats.load()->get()->getInput() + 1) / stats.load()->get()->getTotalInputs();
				ImGui::Text("Input: %d / %d", stats.load()->get()->getInput(), stats.load()->get()->getTotalInputs());
				ImGui::ProgressBar(inputProgress, ImVec2(-1, 0), "");

				ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "LET: %.0f%ms", stats.load()->get()->lastEpochTime());
				ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "AET: %.0f%ms", stats.load()->get()->averageEpochTime());


				// Optional: approximate epoch time
				//float avgTimePerInput = stats->epochTime.load() / stats->input.load(); // assuming you track epochTime
				//ImGui::Text("Approx. Epoch Time: %.2fs", avgTimePerInput * stats.load()->getTotalInputs());

				ImGui::End();
			}

			// OpenGL rendering
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

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	terminate();
}


void Visualiser::terminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwSetWindowShouldClose(window, true);
	glfwWindowShouldClose(window);
	glfwTerminate();
}