#include "application.h"

void SpacecraftRenderingTools::init( ){

    if (!glfwInit())
    {
        throw std::runtime_error( "Error during initialization of GLFW!" );
    }
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window_ = glfwCreateWindow(windowWidth_, windowHeight_, "SpacecraftRenderingTools", NULL, NULL);

    glfwSetWindowUserPointer(window_, this);

    // Framebuffer resize callback
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* w, int width, int height) {
        auto* app = static_cast<SpacecraftRenderingTools*>(glfwGetWindowUserPointer(w));
        app->onResize(width, height);
    });

    // Mouse move callback
    glfwSetCursorPosCallback(window_, [](GLFWwindow* w, double x, double y) {
        auto* app = static_cast<SpacecraftRenderingTools*>(glfwGetWindowUserPointer(w));
        app->onMouseMove(x, y);
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow* w, int button, int action, int mods) {
        static_cast<SpacecraftRenderingTools*>(glfwGetWindowUserPointer(w))->onMouseButton(button, action, mods);
    });

    // Scroll callback (zoom)
    glfwSetScrollCallback(window_, [](GLFWwindow* w, double x, double y) {
        auto* app = static_cast<SpacecraftRenderingTools*>(glfwGetWindowUserPointer(w));
        app->onScroll(x, y);
    });

    if (!window_)
    {
        glfwTerminate();
        throw std::runtime_error( "Error during creation of window!" );
    }

    glfwMakeContextCurrent(window_);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Error, failed to initialize GLAD");
    }

    // initialize imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    renderer_.init( );

}

void SpacecraftRenderingTools::loadMesh( std::string pathToMesh ){
    
    std::ifstream file( pathToMesh );
    if ( !file )
    {
        throw std::runtime_error( "Error, path to mesh does not exist!" );
    }
    std::string line;
    std::vector< std::vector< double > > allData;

    while ( std::getline( file, line ) )
    {
        std::istringstream iss(line);
        double val;
        std::vector<double> values;
        while (iss >> val) { 
            values.push_back(val);
        }
        if (!values.empty()) {
            allData.push_back(values);
        }
    }
    timeSteps_ = allData.size( );
    numberOfTriangles_ = ( allData[ 0 ].size( ) - 13 ) / 10;
    for ( auto timestep: allData )
    {
        spacecraftData_[ float(timestep[ 0 ]) ] = MeshData( timestep );
        times_.push_back( float(timestep[ 0 ]) );
    }
    time_ = float(times_[ 0 ]);
}

void SpacecraftRenderingTools::drawGUI( ){

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("SCRT control panel", nullptr, flagsGUI_);
    ImGui::Text("SCRT control panel");

    ImGui::SetNextItemWidth(400.0f);
    ImGui::InputTextWithHint("save to ", pathToFolder_.data(), pathToFolder_.data(), pathToFolder_.size());
    if (ImGui::Button("Screenshot")){
        takeScreenshot_ = 1;
        ImGui::SameLine();
        ImGui::Text("Screenshot saved!");
    }
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x*0.75f);
    ImGui::SliderFloat("Time [s]", &time_, float(times_[ 0 ]), float(times_[ times_.size()-1 ]));
    if (ImGui::CollapsingHeader("Generic options"))
    {
        ImGui::ColorEdit4("background color", backgroundColor_);
        ImGui::Checkbox("wireframe overlay", &renderer_.wireFrameOverlay_);
    }
    if (ImGui::CollapsingHeader("Properties"))
    {
        const char* items[] = { "Wireframe only", "Self-shadowing", "Temperature" };
        ImGui::Combo("View mode", &setMode_, items, IM_ARRAYSIZE(items), IM_ARRAYSIZE(items));
        setMode( setMode_ );
        if ( setMode_ == 1 || setMode_ == 2 ){
            ImGui::SeparatorText("Colorbar properties");
            
            ImGui::SliderFloat("x position", &xColorbar_, 0.1f, 0.8f);
            ImGui::SliderFloat("y position", &yColorbar_, 0.1f, 0.8f);
            ImGui::SliderFloat("size", &sizeColorbar_, 1.0f, 3.0f);
            ImGui::SliderFloat("fontsize", &fontSize_, 10.0f, 20.0f);
            ImGui::RadioButton("vertical", &verticalColorbar_, 1); ImGui::SameLine();
            ImGui::RadioButton("horizontal", &verticalColorbar_, 0); ImGui::SameLine();
            
        }
        
    }
    ImGui::End();
}

void SpacecraftRenderingTools::updateRender( ) {

    // rotation matrices
    view_ = getViewMatrix();
    projection_ = glm::perspective(
        glm::radians(10.0f),  
        (float)windowWidth_ / (float)windowHeight_,
        0.1f,
        1000.0f
    );

    if (takeScreenshot_){
        glClearColor(backgroundColor_[0], backgroundColor_[1], backgroundColor_[2], backgroundColor_[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderer_.renderMesh( spacecraftData_.at( time_ ), view_, projection_, visualizationMode_ );
        if ( setMode_ == 1 || setMode_ == 2 ){
        drawColorbar( );
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        screenshot( );
        takeScreenshot_ = 0;
    }
    
    glClearColor(backgroundColor_[0], backgroundColor_[1], backgroundColor_[2], backgroundColor_[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderer_.renderMesh( spacecraftData_.at( time_ ), view_, projection_, visualizationMode_ );
    if ( setMode_ == 1 || setMode_ == 2 ){
        drawColorbar( );
    }
    drawGUI( );
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void SpacecraftRenderingTools::mainLoop() {

    while (!glfwWindowShouldClose(window_)) {
       
        updateRender( );

        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    // cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &renderer_.VAO_);
    glDeleteBuffers(1, &renderer_.VBO_);
    glfwTerminate();
}

void SpacecraftRenderingTools::onMouseButton(int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDragging_ = true;
            startRotation_ = rotation_;
            startDragPoint_ = mapToSphere(xpos, ypos);
        } else if (action == GLFW_RELEASE) {
            isDragging_ = false;
        }
    }
    
    
    if (button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isPanning_ = true;
            lastMouseX_ = xpos;
            lastMouseY_ = ypos;
        } else if (action == GLFW_RELEASE) {
            isPanning_ = false;
        }
    }
}

void SpacecraftRenderingTools::onMouseMove(double xpos, double ypos) {

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    if (isDragging_) {
        glm::vec3 currentPoint = mapToSphere(xpos, ypos);
        
        // Get rotation axis and angle
        glm::vec3 axis = glm::cross(startDragPoint_, currentPoint);
        
        if (glm::length(axis) > 0.0001f) {
            axis = glm::normalize(axis);
            float angle = -std::acos(glm::clamp(glm::dot(startDragPoint_, currentPoint), -1.0f, 1.0f));
            
            // Create delta rotation
            glm::quat deltaRotation = glm::angleAxis(angle, axis);
            
            if (!io.WantCaptureMouse) {
                rotation_ = glm::normalize( startRotation_ * deltaRotation);
            }
            else {
                rotation_ = glm::normalize( startRotation_ );
            }
        }
    }
    
    if (isPanning_) {
        float panSensitivity = cameraDistance_ * 0.002f;
        
        float dx = (float)(xpos - lastMouseX_) * panSensitivity;
        float dy = (float)(lastMouseY_ - ypos) * panSensitivity;
        
        // Pan in view space
        glm::mat3 rotationMatrix = glm::mat3_cast(rotation_);
        glm::vec3 right = rotationMatrix * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 up = rotationMatrix * glm::vec3(0.0f, 1.0f, 0.0f);
        
        panOffset_ -= right * dx;
        panOffset_ -= up * dy;
        
        lastMouseX_ = xpos;
        lastMouseY_ = ypos;
    }
}

glm::mat4 SpacecraftRenderingTools::getViewMatrix() {
    // Camera position in world space
    glm::vec3 cameraOffset = glm::vec3(0.0f, 0.0f, cameraDistance_);
    
    // Apply rotation to camera position
    glm::mat4 rotationMatrix = glm::mat4_cast(rotation_);
    glm::vec3 cameraPos = glm::vec3(rotationMatrix * glm::vec4(cameraOffset, 1.0f));
    
    // Target is the pan offset
    glm::vec3 target = panOffset_;
    
    // Up vector rotated
    glm::vec3 up = glm::vec3(rotationMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    
    return glm::lookAt(cameraPos + panOffset_, target, up);
}

void SpacecraftRenderingTools::onScroll(double xoffset, double yoffset) {

    cameraDistance_ -= (float)yoffset * 5.0f;
    if ( cameraDistance_ <= 1.0f )
    {
        cameraDistance_ = 1.0;
    }
    
}

void SpacecraftRenderingTools::onResize(int width, int height) {
    windowWidth_ = width;
    windowHeight_ = height;
    glViewport(0, 0, width, height);
}


int main(void)
{
    std::string pathToMesh = "mesh.txt";

    SpacecraftRenderingTools application( 1280, 960 );
    application.loadMesh( pathToMesh );
    application.mainLoop( );

    return 0;

}

glm::vec3 SpacecraftRenderingTools::mapToSphere(double x, double y) {
    // Normalize screen coordinates to [-1, 1]
    float nx = (2.0f * (float)x / windowWidth_) - 1.0f;
    float ny = 1.0f - (2.0f * (float)y / windowHeight_);
    
    // Compute z coordinate on sphere
    float lengthSquared = nx * nx + ny * ny;
    
    glm::vec3 result;
    if (lengthSquared <= 1.0f) {
        // Point is inside the sphere
        result = glm::vec3(nx, ny, std::sqrt(1.0f - lengthSquared));
    } else {
        // Point is outside the sphere - project onto edge
        float length = std::sqrt(lengthSquared);
        result = glm::vec3(nx / length, ny / length, 0.0f);
    }
    
    return glm::normalize(result);
}

glm::quat SpacecraftRenderingTools::rotationBetweenVectors(const glm::vec3& start, const glm::vec3& end) {
    float dotProduct = glm::dot(start, end);
    
    // Handle edge case where vectors are opposite
    if (dotProduct < -0.999999f) {
        glm::vec3 axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
        if (glm::length(axis) < 0.001f) {
            axis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), start);
        }
        axis = glm::normalize(axis);
        return glm::angleAxis(glm::pi<float>(), axis);
    }
    
    glm::vec3 axis = glm::cross(start, end);
    float s = std::sqrt((1.0f + dotProduct) * 2.0f);
    float invS = 1.0f / s;
    
    return glm::quat(s * 0.5f, axis.x * invS, axis.y * invS, axis.z * invS);
}

// ImGui helpers
void SpacecraftRenderingTools::setMode( int mode ){
    visualizationMode_ = VisualizationMode( mode );
}

ImU32 floatToColor(float t, VisualizationMode mode) {
    
    //basic colors
    std::vector<int> red = {255, 0, 0};
    std::vector<int> white = {255, 255, 255};
    std::vector<int> blue = {0, 0, 255};

    int r, g, b;

    switch(mode){
        case VisualizationMode::SHADOW :{
            r = 255;
            g = int(255 * ( 1.0f - t) );
            b = g;
            break;
        }
        case VisualizationMode::TEMPERATURE :{
            r = int(255 * t);
            g = 0;
            b = int(255 * ( 1.0f - t) );
            break;
        }
    }
    
    return IM_COL32(r, g, b, 255);
}

void SpacecraftRenderingTools::drawColorbar( ) {
    if (verticalColorbar_){
        drawColorbarVertical( );
    }
    else{
        drawColorbarHorizontal( );
    }
}

void SpacecraftRenderingTools::drawColorbarVertical( ) {
    
    float maxValue, minValue;
    const char* title;
    switch(visualizationMode_){
        case VisualizationMode::SHADOW :{
            maxValue = 1.0f;
            minValue = 0.0f;
            title = "f [-]";
            break;
        }
        case VisualizationMode::TEMPERATURE :{
            maxValue = 300.0f;
            minValue = 250.0f;
            title = "T [K]";
            break;
        }
    }
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    float barWidth = 25.0f * sizeColorbar_;
    float barHeight = 200.0f * sizeColorbar_;
    float labelWidth = 60.0f;

    float x = xColorbar_ * windowWidth_;
    float y = yColorbar_ * windowHeight_ ;  
    
    int numSegments = 100;
    float segmentHeight = barHeight / numSegments;
    
    // Draw gradient colorbar
    for (int i = 0; i < numSegments; i++) {
        float t1 = (float)i / numSegments;
        float t2 = (float)(i + 1) / numSegments;
        
        ImU32 color1 = floatToColor(1.0f - t1, visualizationMode_);
        ImU32 color2 = floatToColor(1.0f - t2, visualizationMode_);
        
        ImVec2 p1(x, y + i * segmentHeight);
        ImVec2 p2(x + barWidth, y + (i + 1) * segmentHeight);
        
        drawList->AddRectFilledMultiColor(p1, p2, color1, color1, color2, color2);
    }
    
    // Border
    drawList->AddRect(ImVec2(x, y), 
                      ImVec2(x + barWidth, y + barHeight), 
                      IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.5f);
    
    // Labels
    int numLabels = 5;
    for (int i = 0; i <= numLabels; i++) {
        float t = (float)i / numLabels;
        float value = maxValue - t * (maxValue - minValue);
        float yPos = y + t * barHeight;
        
        // Tick mark
        drawList->AddLine(ImVec2(x + barWidth, yPos),
                          ImVec2(x + barWidth + 5, yPos),
                          IM_COL32(0, 0, 0, 255), 1.5f);
        
        // Label
        char label[32];
        snprintf(label, sizeof(label), "%.1f", value);
        drawList->AddText(ImGui::GetFont(), fontSize_,
                           ImVec2(x + barWidth + 8, yPos - 7), 
                          IM_COL32(0, 0, 0, 255), label);
    }
    
    // Title above colorbar
    ImVec2 textSize = ImGui::CalcTextSize(title);
    drawList->AddText(ImGui::GetFont(), fontSize_,
                      ImVec2(x + (barWidth - textSize.x) / 2.0f, y - 20), 
                      IM_COL32(0, 0, 0, 255), title);
}
void SpacecraftRenderingTools::drawColorbarHorizontal( ) {
    
    float maxValue, minValue;
    const char* title;
    switch(visualizationMode_){
        case VisualizationMode::SHADOW :{
            maxValue = 1.0f;
            minValue = 0.0f;
            title = "f [-]";
            break;
        }
        case VisualizationMode::TEMPERATURE :{
            maxValue = 300.0f;
            minValue = 250.0f;
            title = "T [K]";
            break;
        }
    }
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    
    float barWidth = 200.0f * sizeColorbar_;
    float barHeight = 25.0f * sizeColorbar_;
    float labelWidth = 60.0f;

    float x = xColorbar_ * windowWidth_;
    float y = yColorbar_ * windowHeight_ ;  
    
    int numSegments = 100;
    float segmentWidth= barWidth / numSegments;
    
    // Draw gradient colorbar
    for (int i = 0; i < numSegments; i++) {
        float t1 = (float)i / numSegments;
        float t2 = (float)(i + 1) / numSegments;
        
        ImU32 color1 = floatToColor(t1, visualizationMode_);
        ImU32 color2 = floatToColor(t2, visualizationMode_);
        
        ImVec2 p1(x + i * segmentWidth, y );
        ImVec2 p2(x + (i + 1) * segmentWidth, y +  barHeight);
        
        drawList->AddRectFilledMultiColor(p1, p2, color1, color1, color2, color2);
    }
    
    // Border
    drawList->AddRect(ImVec2(x, y), 
                      ImVec2(x + barWidth, y + barHeight), 
                      IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.5f);
    
    // Labels
    int numLabels = 5;
    for (int i = 0; i <= numLabels; i++) {
        float t = (float)i / numLabels;
        float value = minValue + t * (maxValue - minValue);
        float xPos = x + t * barWidth;
        
        // Tick mark
        drawList->AddLine(ImVec2(xPos, y + barHeight),
                          ImVec2(xPos, y + barHeight + 5),
                          IM_COL32(0, 0, 0, 255), 1.5f);
        
        // Label
        char label[32];
        snprintf(label, sizeof(label), "%.1f", value);
        drawList->AddText(ImGui::GetFont(), fontSize_,
                           ImVec2(xPos - 7, y + barHeight + 8), 
                          IM_COL32(0, 0, 0, 255), label);
    }
    
    // Title above colorbar
    ImVec2 textSize = ImGui::CalcTextSize(title);
    drawList->AddText(ImGui::GetFont(), fontSize_,
                      ImVec2(x + (barWidth - textSize.x) / 2.0f, y - 25.0f), 
                      IM_COL32(0, 0, 0, 255), title);
}


// screenshot
void SpacecraftRenderingTools::screenshot( ){

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    GLsizei nrChannels = 3;
    GLsizei stride = nrChannels * fbWidth;
    stride += (stride % 4) ? (4 - stride % 4) : 0;
    GLsizei bufferSize = stride * fbHeight;
    std::vector<char> buffer(bufferSize);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, fbWidth, fbHeight, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    stbi_flip_vertically_on_write(true);
    stbi_write_png(pathToFolder_.data(), windowWidth_, fbHeight, nrChannels, buffer.data(), stride);
}