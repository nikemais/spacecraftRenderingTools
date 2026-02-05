#include "utilities.h"


std::string loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filepath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

GLuint compileShaderFromFile(const std::string& filepath, GLenum type) {
    std::string code = loadShaderSource(filepath);
    const char* source = code.c_str();
    
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    return shader;
}


void Renderer::checkShaderCompile(GLuint shader){
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
    }
}

void Renderer::checkProgramLink(GLuint program){
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        throw std::runtime_error("Program linking failed: " + std::string(infoLog));
    }
}

void Renderer::init( ){

    // create shaders
    GLuint vertexShader = compileShaderFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShaderFromFile("shaders/fragment_shader.glsl", GL_FRAGMENT_SHADER);

    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);
    checkProgramLink(shaderProgram_);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //
    // MESH BUFFERS
    //
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_); 

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        
    // position attirbute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
                            sizeof(Vertex),
                            (void*)offsetof(Vertex, position_));
    glEnableVertexAttribArray(0);
        
    // shadow attribute
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
                            sizeof(Vertex),
                            (void*)offsetof(Vertex, shadow_));
    glEnableVertexAttribArray(1);

    // temperature attribute
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
                            sizeof(Vertex),
                            (void*)offsetof(Vertex, temperature_));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);


    visualizationModeLocation_ = glGetUniformLocation(shaderProgram_, "visualizationMode");
    wireframeColorLocation_ = glGetUniformLocation(shaderProgram_, "wireframeColor");

}

void Renderer::renderMesh( MeshData& mesh, 
    const glm::mat4& view, 
    const glm::mat4& projection,
    const VisualizationMode visualizationMode ){

    std::vector< Vertex >& vertices = mesh.vertices_;
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.data(),
                 GL_DYNAMIC_DRAW);

    glUseProgram(shaderProgram_);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    switch( visualizationMode ){
        case VisualizationMode::WIREFRAME: {
                
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniform1i(visualizationModeLocation_, int(visualizationMode));
            glUniform4f(wireframeColorLocation_, 0.5f, 0.5f, 0.5f, 1.0f); // grey
            glBindVertexArray(VAO_);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
            glBindVertexArray(0);
        
            break;
        }
        case VisualizationMode::TEMPERATURE:
        case VisualizationMode::SHADOW: {
            // Filled mode (shadow or temperature)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUniform1i(visualizationModeLocation_, int(visualizationMode));
            glBindVertexArray(VAO_);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
            glBindVertexArray(0);
        
            if ( wireFrameOverlay_ ) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glEnable(GL_POLYGON_OFFSET_LINE);
                glPolygonOffset(-1.0f, -1.0f);
            
                glUniform1i(visualizationModeLocation_, int(VisualizationMode::WIREFRAME));
                glUniform4f(wireframeColorLocation_, 0.5f, 0.5f, 0.5f, 1.0f);
                glBindVertexArray(VAO_);
                glDrawArrays(GL_TRIANGLES, 0, vertices.size());
                glBindVertexArray(0);
            
                glDisable(GL_POLYGON_OFFSET_LINE);
            }
            break;
        }
    }
}