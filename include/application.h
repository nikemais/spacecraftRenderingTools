#ifndef APPLICATION_H
#define APPLICATION_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "utilities.h"

class SpacecraftRenderingTools{

public:

    SpacecraftRenderingTools(int width, int height ) : 
        windowWidth_( width ), 
        windowHeight_( height ),
        cameraDistance_(100.0f),
        setMode_( 0 ),
        visualizationMode_( VisualizationMode::WIREFRAME ) { 

            init( );
            glEnable(GL_DEPTH_TEST);
            rotation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            pathToFolder_.resize(256);  // or larger size you need
            std::string path = std::filesystem::current_path().string();
            std::strncpy(pathToFolder_.data(), path.c_str(), pathToFolder_.size() - 1);
            pathToFolder_[pathToFolder_.size() - 1] = '\0';

        };

    void init( );
    void mainLoop( );
    void updateRender( );
    void loadMesh( std::string pathToMesh );


private:

    GLFWwindow* window_;
    std::map< float, MeshData > spacecraftData_;
    int timeSteps_;
    std::vector< float > times_;
    int numberOfTriangles_;
    int windowWidth_;
    int windowHeight_;
    Renderer renderer_;
    VisualizationMode visualizationMode_;
    int setMode_;
    float backgroundColor_[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float time_;

    // colorbar
    float temperatureMax_;
    float temperatureMin_;
    float xColorbar_ = 0.35f;
    float yColorbar_ = 0.1f;
    int verticalColorbar_ = 0;
    float sizeColorbar_ = 2.0f;
    float fontSize_ = 15.0f;

    // Arcball Camera
    float cameraDistance_ = 100.0f;
    glm::quat rotation_;          
    glm::quat startRotation_;     
    glm::vec3 startDragPoint_; 
    glm::vec3 mapToSphere(double x, double y);
    glm::quat rotationBetweenVectors(const glm::vec3& start, const glm::vec3& end);  
    glm::mat4 view_;
    glm::mat4 projection_; 
    
    // mouse state
    bool isDragging_ = false;
    double lastMouseX_ = 0.0;
    double lastMouseY_ = 0.0;
    glm::vec3 panOffset_ = glm::vec3(0.0f);
    bool isPanning_ = false;
    
    // callbacks
    void onMouseButton(int button, int action, int mods);
    void onMouseMove(double xpos, double ypos);
    void onScroll(double xoffset, double yoffset);
    glm::mat4 getViewMatrix();
    void onResize(int width, int height);
    
    // GUI helpers
    void drawGUI( );
    ImGuiWindowFlags flagsGUI_ = ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_NoDecoration
                         | ImGuiWindowFlags_AlwaysAutoResize
                         | ImGuiWindowFlags_NoSavedSettings;
    // helpers
    void setMode( int mode );
    void drawColorbar( );
    void drawColorbarVertical( );
    void drawColorbarHorizontal( );

    // screenshot
    void screenshot( );
    int takeScreenshot_ = 0;
    std::vector<char> pathToFolder_;

};





#endif // APPLICATION_H