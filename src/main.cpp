#include <OpenGLPrj.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Maze.h"
#include "Bfs.h"
#include "Ghost.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <cmath>
#include "glm/ext/scalar_constants.hpp"

const std::string program_name = ("GLSL shaders & uniforms");

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void loadWalls(glm::mat4 &model, unsigned int modelLoc, int vertexColorLocation);
void loadGhost(int vertexColorLocation, Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4, glm::mat4 &model, unsigned int modelLoc) ;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

static const char *vertexShaderSource ="#version 330 core\n"
                                       "layout (location = 0) in vec3 aPos;\n"
                                       "uniform mat4 model;\n"
                                       "uniform mat4 view;\n"
                                       "uniform mat4 projection;"
                                       "void main()\n"
                                       "{\n"
                                       "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                       "}\0";

static const char *fragmentShaderSource = "#version 330 core\n"
                                          "out vec4 FragColor;\n"
                                          "uniform vec4 ourColor;\n"
                                          "void main()\n"
                                          "{\n"
                                          "   FragColor = ourColor;\n"
                                          "}\n\0";

glm::mat4 view = glm::mat4(1.0f);
glm::vec3 cameraPos   = glm::vec3(5.0f, 20.0f,  5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

int main()
{

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, program_name.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float wall[] = {
            // positions         // colors
            0.2f, 0.0f, 1.0f,    // bottom right red
            0.2f,  1.0f, 1.0f,     // top right blue
            0.0f, 0.0f, 1.0f,    // bottom left green
            0.0f,  1.0f, 1.0f,     // top left yellow

            0.0f, 0.0f, 0.0f,    // bottom left back violet
            0.0f,  1.0f, 0.0f,     // top left back pink

            0.2f, 0.0f, 0.0f,    // bottom right back orange
            0.2f,  1.0f, 0.0f,     // top right back cyan

            0.2f, 0.0f, 1.0f,    // bottom right red
            0.2f,  1.0f, 1.0f,     // top right blue

            0.2f,  1.0f, 1.0f,     // top right blue
            0.0f,  1.0f, 1.0f,     // top left yellow
            0.2f,  1.0f, 0.0f,    // top right back cyan
            0.0f,  1.0f, 0.0f,    // top left back pink

            0.2f, 0.0f, 1.0f,    // bottom right red
            0.0f, 0.0f, 1.0f,   // bottom left green
            0.2f, 0.0f, 0.0f,     // bottom right back orange
            0.0f, 0.0f, 0.0f,     // bottom left back violet
    };

    float square[] = {
            // positions         // colors
            0.5f, 0.0f, 0.5f,    // bottom right red
            0.5f,  0.5f, 0.5f,     // top right blue
            0.0f, 0.0f, 0.5f,    // bottom left green
            0.0f,  0.5f, 0.5f,     // top left yellow

            0.0f, 0.0f, 0.0f,    // bottom left back violet
            0.0f,  0.5f, 0.0f,     // top left back pink

            0.5f, 0.0f, 0.0f,    // bottom right back orange
            0.5f,  0.5f, 0.0f,     // top right back cyan

            0.5f, 0.0f, 0.5f,    // bottom right red
            0.5f,  0.5f, 0.5f,     // top right blue

            0.5f,  0.5f, 0.5f,     // top right blue
            0.0f,  0.5f, 0.5f,     // top left yellow
            0.5f,  0.5f, 0.0f,    // top right back cyan
            0.0f,  0.5f, 0.0f,    // top left back pink

            0.5f, 0.0f, 0.5f,    // bottom right red
            0.0f, 0.0f, 0.5f,   // bottom left green
            0.5f, 0.0f, 0.0f,     // bottom right back orange
            0.0f, 0.0f, 0.0f,     // bottom left back violet
    };

    float floor[] = {
            0.0f,0.0f,0.0f,
            10.0f,0.0f,0.0f,
            0.0f,0.0f,10.0f,
            10.0f,0.0f,10.0f
    };

    std::vector<float> vertices;
    for(float f : floor)
        vertices.push_back(f);
    for(float f : wall)
        vertices.push_back(f);
    for(float f : square)
        vertices.push_back(f);



    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // glBindVertexArray(0);


    // bind the VAO (it was already bound, but just to demonstrate): seeing as we only have a single VAO we can
    // just bind it beforehand before rendering the respective triangle; this is another approach.
    glBindVertexArray(VAO);

    //hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    void mouse_callback(GLFWwindow* window, double xpos, double ypos);

    Maze maze = Maze();
    maze.generateMaze();
    maze.printMaze();

    Ghost ghost1 = Ghost(glm::vec3(0.0f,0.0f,0.0f), cols*rows);
    Ghost ghost2 = Ghost(glm::vec3(cols-1,0.0f,rows-1), cols*rows);
    Ghost ghost3 = Ghost(glm::vec3(cols-1,0.0f,0.0f), cols*rows);
    Ghost ghost4 = Ghost(glm::vec3(0.0f,0.0f,rows-1), cols*rows);
    /*BFS bfs = BFS(rows*cols);
    vector<int> path = bfs.get_path(0, 17);*/

    int x=0;
    int z=0;
    float time = 0;
    float ghostMoveSpeed = 3.0f;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        glfwSetCursorPosCallback(window, mouse_callback);

        //path = bfs.get_path(x+z*cols, glm::floor(cameraPos.x)+glm::floor(cameraPos.z)*cols);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //matrices
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, (float)glfwGetTime() * glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        //cameraPos + cameraFront
        view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // retrieve the matrix uniform locations
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // be sure to activate the shader before any calls to glUniform
        glUseProgram(shaderProgram);

        // update shader uniform
        //double timeValue = glfwGetTime();
        //float greenValue = static_cast<float> (sin(timeValue)) / 2.0f + 0.5f;


        //depth
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //color uniform
        int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");

        // render the floor
        glUniform4f(vertexColorLocation, 0.0f,  0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //load walls
        loadWalls(model, modelLoc, vertexColorLocation);

        //load ghost

        loadGhost(vertexColorLocation, ghost1, ghost2, ghost3, ghost4, model, modelLoc);

        //glDrawArrays(GL_TRIANGLE_STRIP, num_angles*2+2,4);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void loadGhost(int vertexColorLocation, Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4, glm::mat4 &model, unsigned int modelLoc) {
    //red
    glUniform4f(vertexColorLocation, 1.0f,  0.0f, 0.0f, 1.0f);
    model = glm::mat4(1.0f);
    ghost1.move(deltaTime, floor(cameraPos.x) + floor(cameraPos.z) * cols);
    model = glm::translate(model, glm::vec3( ghost1.position.x + 0.3f, 0.0f, ghost1.position.z + 0.3f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLE_STRIP, 22, 10);
    glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 36, 4);
    //blue
    glUniform4f(vertexColorLocation, 0.0f,  0.0f, 1.0f, 1.0f);
    model = glm::mat4(1.0f);
    ghost2.move(deltaTime, floor(cameraPos.x) + floor(cameraPos.z) * cols);
    model = glm::translate(model, glm::vec3( ghost2.position.x + 0.3f, 0.0f, ghost2.position.z + 0.3f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLE_STRIP, 22, 10);
    glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 36, 4);
    //green
    glUniform4f(vertexColorLocation, 0.0f,  1.0f, 0.0f, 1.0f);
    model = glm::mat4(1.0f);
    ghost3.move(deltaTime, floor(cameraPos.x) + floor(cameraPos.z) * cols);
    model = glm::translate(model, glm::vec3( ghost3.position.x + 0.3f, 0.0f, ghost3.position.z + 0.3f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLE_STRIP, 22, 10);
    glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 36, 4);
    //yellow
    glUniform4f(vertexColorLocation, 0.0f,  1.0f, 1.0f, 1.0f);
    model = glm::mat4(1.0f);
    ghost4.move(deltaTime, floor(cameraPos.x) + floor(cameraPos.z) * cols);
    model = glm::translate(model, glm::vec3( ghost4.position.x + 0.3f, 0.0f, ghost4.position.z + 0.3f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLE_STRIP, 22, 10);
    glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 36, 4);
}

void loadWalls(glm::mat4 &model, unsigned int modelLoc, int vertexColorLocation) {
    glUniform4f(vertexColorLocation, 0.0f, 0.75f, 1.0f, 1.0f);
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            if(grid[i][j].wallRight){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)(j+1), 0, (float)i));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLE_STRIP, 4, 10);
                glDrawArrays(GL_TRIANGLE_STRIP, 14, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 18, 4);
            }
            if(grid[i][j].wallLeft){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)(j), 0, (float)i));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLE_STRIP, 4, 10);
                glDrawArrays(GL_TRIANGLE_STRIP, 14, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 18, 4);
            }
            if(grid[i][j].wallUp){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)(j)+0.2f, 0, (float)i+0.2));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLE_STRIP, 4, 10);
                glDrawArrays(GL_TRIANGLE_STRIP, 14, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 18, 4);
            }
        }
    }
    for(int j=0;j<cols;j++){
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((float)(j)+0.2f, 0, rows));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 10);
        glDrawArrays(GL_TRIANGLE_STRIP, 14, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 18, 4);
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    float cameraSpeed = 4.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
bool firstMouse = true;
float lastX = SCR_WIDTH/2, lastY = SCR_HEIGHT/2;
float yaw = -90.0f;
float pitch = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}