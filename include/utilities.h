#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>
#include <algorithm>

#include <glad.h>     
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>         
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// base structs and enums
struct Vertex{

    glm::vec3 position_;
    float shadow_;
    float temperature_;

};

struct MeshData{

    MeshData( ) = default;

    MeshData( std::vector< double >& mesh ){

        // sun position
        glm::vec3 sunPositionInertial = glm::vec3( float(mesh[ 1 ]), float(mesh[ 2 ]), float(mesh[ 3 ]) );
        glm::mat3 rotationMatrix = glm::mat3( 
            glm::vec3( float(mesh[ 4 ]), float(mesh[ 7 ]), float(mesh[ 10 ]) ),
            glm::vec3( float(mesh[ 5 ]), float(mesh[ 8 ]), float(mesh[ 11 ]) ),
            glm::vec3( float(mesh[ 6 ]), float(mesh[ 9 ]), float(mesh[ 12 ]) )
        );
        sunPosition_ = rotationMatrix * sunPositionInertial;

        // loading
        int numberOfTriangles = ( mesh.size( ) - 13 ) / 10;
        int trianlgeStartIndex = 13 + numberOfTriangles;
        int colorStartIndex = 13;
        for ( int i = 0; i<numberOfTriangles; i++ )
        {
            float temperature = float(i) / float(numberOfTriangles);
            for ( int j = 0; j<3; j++ ){

                glm::vec3 position = glm::vec3(
                    float(mesh[ trianlgeStartIndex + 9*i + 3*j]), 
                    float(mesh[ trianlgeStartIndex + 1 + 9*i + 3*j]), 
                    float(mesh[ trianlgeStartIndex + 2 + 9*i + 3*j]) );
                
                vertices_.push_back( {position, float(mesh[colorStartIndex + i]), temperature } );
            }                
 
        }

    };

    std::vector< Vertex > vertices_;
    glm::vec3 sunPosition_;

    double tempMax_;
    double tempMin_;

};

enum class VisualizationMode {
    WIREFRAME = 0,
    SHADOW = 1,
    TEMPERATURE = 2
};

// helper functions for checking shaders
std::string loadShaderSource(const std::string& filepath);

GLuint compileShaderFromFile(const std::string& filepath, GLenum type);

// renderer

class Renderer {

public:
    // required for buffers (ids for objects)
    GLuint shaderProgram_ = 0;
    GLuint VAO_ = 0;
    GLuint VBO_ = 0;
    GLuint visualizationModeLocation_ = 0;
    GLuint wireframeColorLocation_ = 0;
    bool wireFrameOverlay_ = true;

    void init( );
    void renderMesh( MeshData& mesh, 
        const glm::mat4& view, 
        const glm::mat4& projection,
        const VisualizationMode visualizationMode );
    void checkShaderCompile(GLuint shader);
    void checkProgramLink(GLuint program);

};

#endif //UTILITIES_H