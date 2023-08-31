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
#include "Shader.h"
#include <ft2build.h>
#include FT_FREETYPE_H


const std::string program_name = ("GLSL shaders & uniforms");

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void loadWalls(glm::mat4 &model, unsigned int modelLoc, int vertexColorLocation);
void loadGhost(int vertexColorLocation, Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4, glm::mat4 &model, unsigned int modelLoc);
void loadCoins(glm::mat4 &model, unsigned int modelLoc, int vertexColorLocation);
void ghostCollision(Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4);
void pickupsCollision();
void ghostScared(Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4);
void RenderText(Shader shaderProgram,std::string text, float x, float y, float scale, glm::vec3 color);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

glm::mat4 view = glm::mat4(1.0f);
glm::vec3 cameraPos   = glm::vec3(5.0f, 20.0f,  5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

int points = 0;
float timer = 0;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
unsigned int VBO2, VAO2;

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader mainShader = Shader("../../shaders/main.vert","../../shaders/main.frag");
    Shader textShader = Shader("../../shaders/text.vert","../../shaders/text.frag");

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // find path to font
    std::string font_name = "C:\\Users\\drist\\Desktop\\Github\\Opengl\\3D-Pacman\\fonts\\arial.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                    texture,
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float wall[] = {
            // positions         // colors
            0.1f, 0.0f, 1.0f,    // bottom right red
            0.1f,  1.0f, 1.0f,     // top right blue
            0.0f, 0.0f, 1.0f,    // bottom left green
            0.0f,  1.0f, 1.0f,     // top left yellow

            0.0f, 0.0f, 0.0f,    // bottom left back violet
            0.0f,  1.0f, 0.0f,     // top left back pink

            0.1f, 0.0f, 0.0f,    // bottom right back orange
            0.1f,  1.0f, 0.0f,     // top right back cyan

            0.1f, 0.0f, 1.0f,    // bottom right red
            0.1f,  1.0f, 1.0f,     // top right blue

            0.1f,  1.0f, 1.0f,     // top right blue
            0.0f,  1.0f, 1.0f,     // top left yellow
            0.1f,  1.0f, 0.0f,    // top right back cyan
            0.0f,  1.0f, 0.0f,    // top left back pink

            0.1f, 0.0f, 1.0f,    // bottom right red
            0.0f, 0.0f, 1.0f,   // bottom left green
            0.1f, 0.0f, 0.0f,     // bottom right back orange
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
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);
    glBindVertexArray(VAO);

    //for ui text
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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

        // be sure to activate the shader before any calls to glUniform
        mainShader.use();
        // retrieve the matrix uniform locations
        unsigned int modelLoc = glGetUniformLocation(mainShader.ID, "model");
        unsigned int viewLoc = glGetUniformLocation(mainShader.ID, "view");
        unsigned int projLoc = glGetUniformLocation(mainShader.ID, "projection");
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

        //depth
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //color uniform
        int vertexColorLocation = glGetUniformLocation(mainShader.ID, "ourColor");

        // render the floor
        glUniform4f(vertexColorLocation, 0.0f,  0.0f, 0.0f, 1.0f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //load walls
        loadWalls(model, modelLoc, vertexColorLocation);

        //load ghost
        loadGhost(vertexColorLocation, ghost1, ghost2, ghost3, ghost4, model, modelLoc);

        //load coins
        loadCoins(model, modelLoc, vertexColorLocation);

        ghostScared(ghost1, ghost2, ghost3, ghost4);
        pickupsCollision();
        ghostCollision(ghost1, ghost2, ghost3, ghost4);

        //ui text

        RenderText(textShader, "Points: "+ to_string(points), 0, SCR_HEIGHT-50, 1.0f, glm::vec3(0.0, 0.0f, 0.0f));
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

void ghostScared(Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4) {
    if(timer > 0){
        timer-=deltaTime;
        ghost1.isScared = true;
        ghost2.isScared = true;
        ghost3.isScared = true;
        ghost4.isScared = true;
    }
    if(timer < 0){
        timer = 0;
        ghost1.isScared = false;
        ghost2.isScared = false;
        ghost3.isScared = false;
        ghost4.isScared = false;
    }
}

void pickupsCollision() {
    if(grid[floor(cameraPos.z)][floor(cameraPos.x)].hasCoin){
        grid[floor(cameraPos.z)][floor(cameraPos.x)].hasCoin = false;
        points += 10;
        cout<<points<<endl;
    }
    if(grid[floor(cameraPos.z)][floor(cameraPos.x)].hasPellet){
        grid[floor(cameraPos.z)][floor(cameraPos.x)].hasPellet = false;
        points += 10;
        cout<<points<<endl;
        timer = 5;
    }
}

void ghostCollision(Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4) {
    if(floor(cameraPos.x) == floor(ghost1.position.x) && floor(cameraPos.z) == floor(ghost1.position.z)){
        if(ghost1.isScared){
            ghost1 = Ghost(glm::vec3(0.0f,0.0f,0.0f), cols*rows);
        }
    }
    if(floor(cameraPos.x) == floor(ghost2.position.x) && floor(cameraPos.z) == floor(ghost2.position.z)){
        if(ghost2.isScared){
            ghost2 = Ghost(glm::vec3(cols-1,0.0f,rows-1), cols*rows);
        }
    }
    if(floor(cameraPos.x) == floor(ghost3.position.x) && floor(cameraPos.z) == floor(ghost3.position.z)){
        if(ghost3.isScared){
            ghost3 = Ghost(glm::vec3(cols-1,0.0f,0.0f), cols*rows);
        }
    }
    if(floor(cameraPos.x) == floor(ghost4.position.x) && floor(cameraPos.z) == floor(ghost4.position.z)){
        if(ghost4.isScared){
            ghost4 = Ghost(glm::vec3(0.0f,0.0f,rows-1), cols*rows);
        }
    }
}

void loadCoins(glm::mat4 &model, unsigned int modelLoc, int vertexColorLocation) {
    for(int i=0; i < rows; i++){
        for(int j=0;j<cols;j++){
            if(grid[i][j].hasCoin){
                glUniform4f(vertexColorLocation, 1.0f,  1.0f, 0.0f, 1.0f);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3( j + 0.5f, 0.1f, i + 0.5f));
                model = glm::scale(model, glm::vec3( 0.3f, 0.3f, 0.3f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLE_STRIP, 22, 10);
                glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 36, 4);
            }
            if(grid[i][j].hasPellet){
                glUniform4f(vertexColorLocation, 1.0f,  0.5f, 0.0f, 1.0f);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3( j + 0.5f, 0.1f, i + 0.5f));
                model = glm::scale(model, glm::vec3( 0.3f, 0.3f, 0.3f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLE_STRIP, 22, 10);
                glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 36, 4);
            }
        }
    }
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
                model = glm::translate(model, glm::vec3((float)(j)+0.1f, 0, (float)i+0.1));
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

void RenderText(Shader shaderProgram,std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state
    shaderProgram.use();
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO2);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO2);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
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