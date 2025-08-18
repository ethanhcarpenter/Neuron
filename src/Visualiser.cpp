#include "Visualiser.h"





#pragma region Visualiser-Setup
Visualiser::Visualiser() :isSetup(false), isNNRunning(false) {};
void Visualiser::setup(const char* name, int targetMonitorIndex, std::shared_ptr<Statistics>* s, int windowWidth, int windowHeight) {
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
		| ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	confirmedLayerSizes = false;
	lastUpdate = glfwGetTime();
	currentConnection = 0;
	startingAnimation = true;
	usableHeight = getTabContentHeight();
	io = ImGui::GetIO();
	fontDefault = io.Fonts->AddFontFromFileTTF("lib/fonts/vs.ttf", 20.0f);
	fontLarge = io.Fonts->AddFontFromFileTTF("lib/fonts/vs.ttf", 28.0f);
}
void Visualiser::postSetupLogic() {
	generateNeuronPositions(layers, windowDimensions.first, usableHeight);
	neuronRadius = calculateNeuronRadius(usableHeight, 0.01f);
	calculateConnectionCount();
}
int Visualiser::getTabContentHeight() {
	int usableHeight = 0;
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
const bool Visualiser::isSettingUp() { return !isSetup; }
GLFWwindow* Visualiser::getWindow() { return window; }
void Visualiser::terminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwSetWindowShouldClose(window, true);
	glfwWindowShouldClose(window);
	glfwTerminate();
}
#pragma endregion



#pragma region Neurons
void Visualiser::drawNeurons() {
	glColor3f(0.0f, 0.0f, 1.0f);
	for (auto& layer : positions) {
		int a = 12;
		for (auto& neuron : layer) {
			drawCircle(neuron.first, neuron.second, neuronRadius, 64);
		}
	}
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
void Visualiser::generateNeuronPositions(const std::vector<int>& layers, float width, float height) {
	if (layers.empty()) { return; }
	float layerSpacingX = width / (layers.size() + 1);
	for (size_t i = 0; i < layers.size(); i++) {
		std::vector<std::pair<float, float>> layerPositions;
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
const float Visualiser::calculateNeuronRadius(float height, float margin) {
	int maxNeurons = *max_element(layers.begin(), layers.end());
	float spacingY = height / (maxNeurons + 1);
	return (spacingY / 2.0f) - margin;
}
#pragma endregion



#pragma region Connections-Class

#pragma region Initalise
Connection::Connection(float fX, float fY, float tX, float tY, float w) {
	fromX = fX;
	fromY = fY;
	toX = tX;
	toY = tY;
	weight = w;
}
#pragma endregion


#pragma region Get
const float Connection::getFromX() { return fromX; }
const float Connection::getFromY() { return fromY; }
const float Connection::getToX() { return toX; }
const float Connection::getToY() { return toY; }
#pragma endregion


#pragma region Set
void Connection::setWeight(float w) { weight = w; }
const float Connection::getWeight() { return weight; }
#pragma endregion

#pragma endregion



#pragma region Connections
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
const int Visualiser::getCurrentConnection() { return currentConnection; }
std::vector<Connection>& Visualiser::getConnections() { return connections; }
std::tuple<float, float, float, float> Visualiser::generateColour(float weight) {
	float opacity = std::min(1.0f, abs(weight));
	if (weight < 0) {
		return { 1.0f, 0.0f, 0.0f, opacity };
	}
	else {
		return { 0.0f, 1.0f, 0.0f, opacity };
	}
}
void Visualiser::calculateConnectionCount() {
	calculatedConnectionCount = 0;
	for (int i = 0; i < layers.size() - 1; i++) {
		int product = layers[i] * layers[i + 1];
		calculatedConnectionCount += product;
	}
}
std::string Visualiser::generateConnectionUID(int fromLayer, int from, int to) {
	std::string uid = "from layer:" + std::to_string(fromLayer) + " node: " + std::to_string(from) + " To layer: " + std::to_string(fromLayer + 1) + " node: " + std::to_string(to);
	return uid;
}
const float Visualiser::getConnectionWeight(int fromLayer, int from, int to, float weight) {
	std::string uid = generateConnectionUID(fromLayer, from, to);
	auto it = connectionsIndexes.find(uid);
	int index = it->second;
	return connections.at(index).getWeight();
}
void Visualiser::updateConnection(int fromLayer, int from, int to, float weight) {

	std::string uid = generateConnectionUID(fromLayer, from, to);
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
void Visualiser::addConnectionIndex(int fromLayer, int from, int to) {
	std::string uid = "";
	uid += std::to_string(fromLayer);
	uid += std::to_string(from);
	uid += std::to_string(to);
	connectionsIndexes[uid] = static_cast<int>(connections.size() - 1);
}
#pragma endregion



#pragma region NN-Tab
void Visualiser::drawNeuralNetwork(int winWidth, int winHeight, std::shared_ptr<std::shared_mutex>* statsMutex) {
	drawImGuiBriefNNStats(winWidth, winHeight, statsMutex);

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
void Visualiser::drawImGuiBriefNNStats(int winWidth, int winHeight, std::shared_ptr<std::shared_mutex>* statsMutex) {
	std::shared_lock<std::shared_mutex> lock(*statsMutex->get());
	ImGui::PushFont(fontDefault);
	ImGui::SetNextWindowPos(ImVec2(winWidth - 320, winHeight - usableHeight), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Always);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.7f));

	ImGui::Begin("Training Overlay", nullptr,
		windowFlags | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);

	ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "Epoch %d", stats.load()->get()->getEpoch());

	float inputProgress = static_cast<float>(stats.load()->get()->getInput() + 1) / stats.load()->get()->getTotalInputs();
	ImGui::Text("Input: %d / %d", stats.load()->get()->getInput(), stats.load()->get()->getTotalInputs());
	ImGui::ProgressBar(inputProgress, ImVec2(-1, 0), "");

	ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "LET: %.0f ms", stats.load()->get()->getLastEpochTime());
	ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "AET: %.0f ms", stats.load()->get()->getAverageEpochTime());
	ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "Accuracy: %.0f%%", stats.load()->get()->getTestAccuracy());

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	ImGui::PopFont();
}
#pragma endregion



#pragma region Console-Tab
void Visualiser::drawConsole(int winWidth, int winHeight, std::shared_ptr<std::shared_mutex>* statsMutex) {
	std::shared_lock<std::shared_mutex> lock(*statsMutex->get());
	ImGui::PushFont(fontLarge);
	bool running = stats.load()->get()->getRunning();

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	if (confirmedLayerSizes) { ImGui::BeginDisabled(); }
	std::vector<int> layerArch = drawLayerInputs();
	if (confirmedLayerSizes) { ImGui::EndDisabled(); }

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	if (isNNRunning) { ImGui::BeginDisabled(); }
	std::tuple<int, float> numericInputs = drawNumericInputs();
	if (isNNRunning) { ImGui::EndDisabled(); }

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	if (isNNRunning) { ImGui::BeginDisabled(); }
	std::string activation = drawActivationInput();
	if (isNNRunning) { ImGui::EndDisabled(); }

	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	bool confirmInputs = drawButton(
		ButtonStyle{
			ImVec2((ImGui::GetContentRegionAvail()[0]), 50),
			"Confirm",
			ImVec4(0.2f, 0.8f, 0.2f, 1.0f),
			ImVec4(0.3f, 0.9f, 0.3f, 1.0f),
			ImVec4(0.1f, 0.7f, 0.1f, 1.0f),
		});
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	drawButton(
		ButtonStyle{
			ImVec2((ImGui::GetContentRegionAvail()[0]), 50),
			"Reset",
			ImVec4(0.9f, 0.5f, 0.1f, 1.0f),
			ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
			ImVec4(0.8f, 0.4f, 0.0f, 1.0f),
		});
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	bool pressed = drawButton(
		ButtonStyle{
			ImVec2(ImGui::GetContentRegionAvail()[0], 50),
			"Start Training",
			ImVec4(0.8f, 0.2f, 0.2f, 1.0f),
			ImVec4(0.9f, 0.3f, 0.3f, 1.0f),
			ImVec4(0.7f, 0.1f, 0.1f, 1.0f),
		},
		ButtonStyle{
			ImVec2(ImGui::GetContentRegionAvail()[0], 50),
			"Stop Training",
			ImVec4(0.2f, 0.8f, 0.2f, 1.0f),
			ImVec4(0.3f, 0.9f, 0.3f, 1.0f),
			ImVec4(0.1f, 0.7f, 0.1f, 1.0f),
		}, running);

	if (confirmInputs) { updateStats(statsMutex, running, layerArch, numericInputs, activation); }
	if (pressed) {
		stats.load()->get()->setRunning(!running);
		isNNRunning = !isNNRunning;
	}
	ImGui::PopFont();

}
bool Visualiser::drawButton(ButtonStyle defaultStyle, ButtonStyle constantPressedStyle, bool lock) {
	bool confirm = false;
	if (lock) {
		ImGui::PushStyleColor(ImGuiCol_Button, constantPressedStyle.colour);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, constantPressedStyle.hoverColour);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, constantPressedStyle.activeColour);
		confirm = (ImGui::Button(constantPressedStyle.text.c_str(), constantPressedStyle.dimensions));
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, defaultStyle.colour);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, defaultStyle.hoverColour);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, defaultStyle.activeColour);
		confirm = (ImGui::Button(defaultStyle.text.c_str(), defaultStyle.dimensions));
	}

	ImGui::PopStyleColor(3);
	return confirm;
}
std::vector<int> Visualiser::drawLayerInputs() {
	ImGui::Text("Network Architecture");

	static int numLayers = 4;
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(100, 100, 250, 100));
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
	ImGui::InputInt("Number of Layers", &numLayers);
	if (numLayers < 2) numLayers = 2;
	if (numLayers > 16) numLayers = 63;
	ImGui::PopStyleColor(2);

	static std::vector<int> layers = { 49, 32,64, 10 };
	if (layers.size() != numLayers) { layers.resize(numLayers, 1); }

	float baseLayerHeight = 30.0f;
	float maxChildHeight = 300.0f;
	float childHeight = std::min(baseLayerHeight * numLayers + 40.0f, maxChildHeight);

	ImGui::BeginChild("NetworkArch", ImVec2(0, childHeight), true);
	for (int i = 0; i < numLayers; i++) {
		char label[64];
		if (i == 0) snprintf(label, sizeof(label), "Input Layer ");
		else if (i == numLayers - 1) snprintf(label, sizeof(label), "Output Layer ");
		else snprintf(label, sizeof(label), "Hidden Layer %d", i);

		ImGui::PushItemWidth(150);
		ImGui::InputInt(label, &layers[i]);
		if (layers[i] < 1) layers[i] = 1;
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	return layers;
}
std::tuple<int, float> Visualiser::drawNumericInputs() {
	ImGui::BeginChild("TrainingParams", ImVec2(0, 150), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
	ImGui::Text("Training Parameters");
	static int epochs = 10000;
	static float learningRate = 0.005f;
	ImGui::InputInt("Maximum Epochs", &epochs, 10, 100);
	ImGui::InputFloat("Learning Rate", &learningRate, 0.001f, 0.01f, "%.5f");
	ImGui::EndChild();
	return std::make_tuple(epochs, learningRate);
}
std::string Visualiser::drawActivationInput() {
	ImGui::BeginChild("Activation", ImVec2(0, 90), true);
	ImGui::Text("Activation Function");
	static int activationType = 1;
	const char* activations[] = { "relu", "sigmoid", "tanh", "linear" };
	ImGui::Combo("Type", &activationType, activations, IM_ARRAYSIZE(activations));
	ImGui::EndChild();
	return activations[activationType];
}
void Visualiser::updateStats(std::shared_ptr<std::shared_mutex>* statsMutex, bool r, std::vector<int> ls, std::tuple<int, float> ni, std::string at) {
	auto& s = *stats.load()->get();
	s.setLayerSizes(ls);
	layers = ls;
	s.setMaxEpochs(get<0>(ni));
	s.setLearningRate(get<1>(ni));
	s.setActivationType(at);
	if (!confirmedLayerSizes) {
		confirmedLayerSizes = true;
		isSetup = true;
		postSetupLogic();
	}
	else { s.setNeuralNetworkNeedsUpdating(true); }
}
#pragma endregion



#pragma region Main-Loop
void Visualiser::mainLoop(std::shared_ptr<std::shared_mutex>* statsMutex) {

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

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		int tabShowingIndex = 0;
		if (ImGui::Begin("Main Tabs", nullptr, windowFlags | ImGuiWindowFlags_NoBackground)) {
			if (ImGui::BeginTabBar("Tabs")) {
				if (ImGui::BeginTabItem("Starting Tab")) {
					tabShowingIndex = 0;
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Console")) {
					tabShowingIndex = 1;
					drawConsole(winWidth, winHeight, statsMutex);
					ImGui::EndTabItem();
				}
				if (isNNRunning && ImGui::BeginTabItem("Neural Network")) {
					tabShowingIndex = 2;
					ImGui::EndTabItem();
				}


				ImGui::EndTabBar();
			}
			ImGui::End();
		}
		if (tabShowingIndex == 2) { drawNeuralNetwork(winWidth, winHeight, statsMutex); }

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	terminate();
}
#pragma endregion
















