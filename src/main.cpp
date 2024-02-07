#include <OpenGLPrj.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Maze.h"
#include "Ghost.h"

#include <iostream>
#include <cmath>
#include <vector>
#include "Shader.h"
#include <ft2build.h>
#include <Model.h>
#include <map>
#include <assimp/Importer.hpp>
#include FT_FREETYPE_H


const std::string program_name = ("3D PACMAN");

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void loadWalls(glm::mat4 &model, Shader &shader, Model &wall);
void loadGhost(Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4,
               Model &blinky, Model &pinky, Model &inky, Model &clyde, Model &scared, Shader &shader, glm::mat4 &model) ;

void loadCoins(glm::mat4 &model, Shader &shader, Model &coin, Model &pellet);
void ghostCollision(Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4);
void pickupsCollision();
void ghostScared(Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4);
void RenderText(Shader shaderProgram,std::string text, std::string text_position_x, float y, float scale, glm::vec3 color);
float distance(float x1, float x2);
void startGame();

// settings
const unsigned int SCR_WIDTH = 985;
const unsigned int SCR_HEIGHT = 700;

glm::mat4 view = glm::mat4(1.0f);
//position camera in the centre of the maze
glm::vec3 cameraPos   = glm::vec3(cols/2+0.5f, 0.5f,  rows/2+0.5f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

int points = 0;
float timer = 0;
float wallSize = 0.3f;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
unsigned int VBO, VAO;

//Game objects
Ghost ghost1;
Ghost ghost2;
Ghost ghost3;
Ghost ghost4;
Maze maze;

bool BFS::GAMEOVER = false;

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
    glfwSetCursorPosCallback(window, mouse_callback);
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

    Shader textShader = Shader("../../../shaders/text.vert","../../../shaders/text.frag");
    Shader minimapShader("../../../shaders/minimap.vert", "../../../shaders/minimap.frag");
    Shader modelShader("../../../shaders/model_loading.vert", "../../../shaders/model_loading.frag");
    Shader lightingModelShader("../../../shaders/lighting.vert", "../../../shaders/lighting.frag");



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
    std::string font_name = "../../../fonts/arial.ttf";
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
    float minimap[] = {
            0.5f, 1.0f, 0.0f, 1.0f,
            0.5f,0.5f, 0.0f, 0.0f,
            1.0f,0.5f, 1.0f, 0.0f,

            0.5f, 1.0f, 0.0f, 1.0f,
            1.0f,0.5f, 1.0f, 0.0f,
            1.0f,1.0f, 1.0f, 1.0f,
    };

    //for ui text
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //for minimap
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(minimap), &minimap, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    //create frame buffer for minimap
    GLuint fbo;
    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    //create texture for minimap
    GLuint img;
    glGenTextures(1, &img);
    glBindTexture(GL_TEXTURE_2D, img);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmapEXT(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, img, 0);
    //create render buffer for minimap
    GLuint depthbuffer;
    glGenRenderbuffersEXT(1, &depthbuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    //hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Model floor("../../../blender objects/colliders/pacman_floor.obj");
    Model wall("../../../blender objects/colliders/pacman_wall.obj");
    Model coin("../../../blender objects/pickups/pacman_ball.obj");
    Model pellet("../../../blender objects/pickups/pacman_powerup.obj");

    Model pacman("../../../blender objects/ghosts/player.obj");
    Model blinky("../../../blender objects/ghosts/pacman_ghost_red.obj");
    Model pinky("../../../blender objects/ghosts/pacman_ghost_pink.obj");
    Model inky("../../../blender objects/ghosts/pacman_ghost_blue.obj");
    Model clyde("../../../blender objects/ghosts/pacman_ghost_orange.obj");
    Model scaredGhost("../../../blender objects/ghosts/pacman_ghost_scared.obj");

    startGame();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        //FIRST RENDER
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);

        //cameraPos + cameraFront
        glm::vec3 mCameraPos;
        float offset;
        if(SCR_WIDTH < SCR_HEIGHT){
            mCameraPos = glm::vec3(5.0f, 16.0f, 5.0f);
        }else{
            mCameraPos = glm::vec3(5.0f, 13.0f, 5.0f);
        }

        glm::vec3 mCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);;
        glm::mat4 view2 = glm::lookAt(mCameraPos, mCameraFront + mCameraPos, cameraUp);

        glm::mat4 projection2;
        projection2 = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        projection2 = glm::rotate(projection2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // retrieve the matrix uniform locations
        // pass them to the shaders (3 different ways)
        lightingModelShader.use();
        lightingModelShader.setMat4("model", model);
        lightingModelShader.setMat4("view", view2);
        lightingModelShader.setMat4("projection", projection2);

        floor.Draw(lightingModelShader);

        modelShader.use();
        modelShader.setMat4("model", model);
        modelShader.setMat4("view", view2);
        modelShader.setMat4("projection", projection2);

        //load player
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cameraPos.x, 1.0f, cameraPos.z));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        modelShader.setMat4("model", model);
        pacman.Draw(modelShader);

        //load walls
        loadWalls(model, modelShader, wall);

        //load ghost
        loadGhost(ghost1, ghost2, ghost3, ghost4,
                  blinky, pinky, inky, clyde, scaredGhost, modelShader, model);

        //load coins
        loadCoins(model, modelShader, coin, pellet);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        //FIRST RENDER END


        //SECOND RENDER
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lightingModelShader.use();

        model = glm::mat4(1.0f);

        view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
        projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        lightingModelShader.use();

        // pass them to the shaders (3 different ways)
        lightingModelShader.setMat4("model", model);
        lightingModelShader.setMat4("view", view);
        lightingModelShader.setMat4("projection", projection);


        //lighting
        lightingModelShader.use();
        lightingModelShader.setVec3("viewPos", cameraPos);
        lightingModelShader.setFloat("material.shininess", 32.0f);

        for(int i=0; i < rows; i++){
            for(int j=0;j< cols;j++){
                if(grid[i][j].hasCoin){
                    lightingModelShader.setVec3("pointLights["+std::to_string(i*10+j)+"].position", glm::vec3( j + 0.5f, 0.15f, i + 0.5f));
                    lightingModelShader.setVec3("pointLights["+std::to_string(i*10+j)+"].diffuse", 30.0f, 30.0f, 30.0f);
                    lightingModelShader.setFloat("pointLights["+std::to_string(i*10+j)+"].constant", 1.0f);
                    lightingModelShader.setFloat("pointLights["+std::to_string(i*10+j)+"].linear", 100.0f);
                    lightingModelShader.setFloat("pointLights["+std::to_string(i*10+j)+"].quadratic", 500.0f);
                }else{
                    lightingModelShader.setVec3("pointLights["+std::to_string(i*10+j)+"].position", glm::vec3( j + 0.5f, 0.15f, i + 0.5f));
                    lightingModelShader.setVec3("pointLights["+std::to_string(i*10+j)+"].diffuse", 0.0f, 0.0f, 0.0f);
                    lightingModelShader.setFloat("pointLights["+std::to_string(i*10+j)+"].constant", 1.0f);
                    lightingModelShader.setFloat("pointLights["+std::to_string(i*10+j)+"].linear", 0.0f);
                    lightingModelShader.setFloat("pointLights["+std::to_string(i*10+j)+"].quadratic", 0.0f);
                }

            }
        }

        // spotLight
        lightingModelShader.setVec3("spotLight.position", cameraPos);
        lightingModelShader.setVec3("spotLight.direction", cameraFront);
        lightingModelShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingModelShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingModelShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingModelShader.setFloat("spotLight.constant", 1.0f);
        lightingModelShader.setFloat("spotLight.linear", 0.09f);
        lightingModelShader.setFloat("spotLight.quadratic", 0.032f);
        lightingModelShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingModelShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(25.0f)));

        floor.Draw(lightingModelShader);

        //load walls
        loadWalls(model, lightingModelShader, wall);

        //load ghost
        modelShader.use();
        modelShader.setMat4("model", model);
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", projection);

        if(!BFS::GAMEOVER){
            ghost1.move(deltaTime, std::floor(cameraPos.x) + std::floor(cameraPos.z) * cols);
            ghost2.move(deltaTime, std::floor(cameraPos.x) + std::floor(cameraPos.z) * cols);
            ghost3.move(deltaTime, std::floor(cameraPos.x) + std::floor(cameraPos.z) * cols);
            ghost4.move(deltaTime, std::floor(cameraPos.x) + std::floor(cameraPos.z) * cols);
        }

        loadGhost(ghost1, ghost2, ghost3, ghost4,
                  blinky, pinky, inky, clyde, scaredGhost, modelShader, model);

        //load coins
        loadCoins(model, modelShader, coin, pellet);

        lightingModelShader.use();
        ghostScared(ghost1, ghost2, ghost3, ghost4);
        pickupsCollision();
        ghostCollision(ghost1, ghost2, ghost3, ghost4);

        if(!BFS::GAMEOVER){
            RenderText(textShader, "Points: "+ to_string(points), "left", SCR_HEIGHT-50, 1.0f, glm::vec3(1.0, 1.0f, 1.0f));
        }else{
            if(points == 1000){
                RenderText(textShader, "CONGRATS", "center", int(SCR_HEIGHT/1.8), 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
                RenderText(textShader, "YOU WON!", "center", int(SCR_HEIGHT/2.5), 1.5f, glm::vec3(1.0, 1.0f, 1.0f));
            }else{
                RenderText(textShader, "GAME OVER", "center", int(SCR_HEIGHT/1.8), 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
                RenderText(textShader, "PRESS ENTER TO RESTART", "center", int(SCR_HEIGHT/2.5), 1.5f, glm::vec3(1.0, 1.0f, 1.0f));
            }
        }

        //MINIMAP
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, img);
        minimapShader.use();

        if(SCR_WIDTH < SCR_HEIGHT){
            offset = (1.0 / ((float)SCR_HEIGHT/(float)SCR_WIDTH) - 1.0) / 2.0;
            minimapShader.setFloat("offset_x",  0.0f);
            minimapShader.setFloat("offset_y",  offset);
        }else{
            offset = (1.0 / ((float)SCR_WIDTH/(float)SCR_HEIGHT) - 1.0) / 2.0;
            minimapShader.setFloat("offset_x",  offset);
            minimapShader.setFloat("offset_y",  0.0f);
        }

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //SECOND RENDER END

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteFramebuffersEXT(1, &fbo);
    glDeleteTextures(1, &img);
    glDeleteRenderbuffers(1, &depthbuffer);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void startGame(){
    maze = Maze();
    maze.generateMaze();
    maze.printMaze();
    points = 0;

    ghost1 = Ghost(glm::vec3(0.0f,0.0f,0.0f), cols*rows);
    ghost2 = Ghost(glm::vec3(cols-1,0.0f,rows-1), cols*rows);
    ghost3 = Ghost(glm::vec3(cols-1,0.0f,0.0f), cols*rows);
    ghost4 = Ghost(glm::vec3(0.0f,0.0f,rows-1), cols*rows);

    cameraPos   = glm::vec3(cols/2+0.5f, 0.5f,  rows/2+0.5f);

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
    }
    if(grid[floor(cameraPos.z)][floor(cameraPos.x)].hasPellet){
        grid[floor(cameraPos.z)][floor(cameraPos.x)].hasPellet = false;
        points += 10;
        timer = 5;
    }
    if(points == 1000){
        BFS::GAMEOVER = true;
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

void loadCoins(glm::mat4 &model, Shader &shader, Model &coin, Model &pellet) {
    for(int i=0; i < rows; i++){
        for(int j=0;j<cols;j++){
            if(grid[i][j].hasCoin){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3( j + 0.5f, 0.1f, i + 0.5f));
                model = glm::scale(model, glm::vec3( 0.08f, 0.08f, 0.08f));
                shader.setMat4("model", model);
                coin.Draw(shader);
            }

            if(grid[i][j].hasPellet){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3( j + 0.5f, 0.1f, i + 0.5f));
                model = glm::scale(model, glm::vec3( 0.08f, 0.08f, 0.08f));
                shader.setMat4("model", model);
                pellet.Draw(shader);
            }
        }
    }
}

void loadGhost(Ghost &ghost1, Ghost &ghost2, Ghost &ghost3, Ghost &ghost4,
               Model &blinky, Model &pinky, Model &inky, Model &clyde, Model &scared, Shader &shader, glm::mat4 &model) {
    float scale = 0.2f;

    //red
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3( ghost1.position.x  + 0.5f, 0.3f, ghost1.position.z + 0.5f));
    model = glm::rotate(model, glm::radians(ghost1.rotation), glm::vec3(0, 1.0f, 0));
    model = glm::scale(model, glm::vec3( scale, scale, scale));
    shader.setMat4("model", model);
    if(ghost1.isScared)
        scared.Draw(shader);
    else
        blinky.Draw(shader);

    //blue
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3( ghost2.position.x + 0.5f, 0.3f, ghost2.position.z + 0.5f));
    model = glm::rotate(model, glm::radians(ghost2.rotation), glm::vec3(0, 1.0f, 0));
    model = glm::scale(model, glm::vec3( scale, scale, scale));
    shader.setMat4("model", model);
    if(ghost2.isScared)
        scared.Draw(shader);
    else
        pinky.Draw(shader);

    //green
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3( ghost3.position.x + 0.5f, 0.3f, ghost3.position.z + 0.5f));
    model = glm::rotate(model, glm::radians(ghost3.rotation), glm::vec3(0, 1.0f, 0));
    model = glm::scale(model, glm::vec3( scale, scale, scale));
    shader.setMat4("model", model);
    if(ghost3.isScared)
        scared.Draw(shader);
    else
        inky.Draw(shader);

    //yellow
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3( ghost4.position.x + 0.5f, 0.3f, ghost4.position.z + 0.5f));
    model = glm::rotate(model, glm::radians(ghost4.rotation), glm::vec3(0, 1.0f, 0));
    model = glm::scale(model, glm::vec3( scale, scale, scale));
    shader.setMat4("model", model);
    if(ghost4.isScared)
        scared.Draw(shader);
    else
        clyde.Draw(shader);
}

void loadWalls(glm::mat4 &model, Shader &shader, Model &wall) {
    float scale = 0.132f;
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            if(grid[i][j].wallRight){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)(j+1), 0.05f, (float)i+0.6f));
                model = glm::scale(model, glm::vec3( scale, scale, scale));
                shader.setMat4("model", model);
                wall.Draw(shader);
            }
            if(grid[i][j].wallLeft){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)(j), 0.05f, (float)i+0.6f));
                model = glm::scale(model, glm::vec3( scale, scale, scale));
                shader.setMat4("model", model);
                wall.Draw(shader);
            }
            if(grid[i][j].wallUp){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)(j)+0.5f, 0.05f, (float)i+0.1));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0));
                model = glm::scale(model, glm::vec3( scale, scale, scale));
                shader.setMat4("model", model);
                wall.Draw(shader);
            }
        }
    }
    for(int j=0;j<cols;j++){
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((float)(j)+0.5f, 0.05f, rows));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0));
        model = glm::scale(model, glm::vec3( scale, scale, scale));
        shader.setMat4("model", model);
        wall.Draw(shader);
    }


}

void RenderText(Shader shaderProgram,std::string text, std::string text_position_x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state
    shaderProgram.use();
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    scale = scale * (SCR_WIDTH / 1000.0f);
    float x;

    // iterate through all characters
    std::string::const_iterator c;

    if(text_position_x == "center")
    {
        float text_width = 0.0f;
        for (c = text.begin(); c != text.end(); c++)
        {
            Character ch = Characters[*c];
            text_width += (ch.Advance >> 6) * scale;
        }
        x = SCR_WIDTH/2.0f - text_width/2.0f;
    }else{
        //text_position_x == left
        x = 1.0f;
    }

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
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
    glm::vec3 camera = glm::vec3(1.0f, 0.0f,  1.0f);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(!BFS::GAMEOVER){
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront * camera;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront * camera;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront * camera, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront * camera, cameraUp)) * cameraSpeed;
    }

    if(BFS::GAMEOVER){
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
            startGame();
    }

    //wall collisions
    if(grid[floor(cameraPos.z)][floor(cameraPos.x)].wallUp && distance(cameraPos.z,glm::floor(cameraPos.z)) <= wallSize)
        cameraPos.z = glm::floor(cameraPos.z)+wallSize;
    if(grid[floor(cameraPos.z)][floor(cameraPos.x)].wallDown && distance(cameraPos.z,glm::floor(cameraPos.z)+1.0f) <= wallSize)
        cameraPos.z = glm::floor(cameraPos.z)+1.0f-wallSize;
    if(grid[floor(cameraPos.z)][floor(cameraPos.x)].wallLeft && distance(cameraPos.x,glm::floor(cameraPos.x)) <= wallSize)
        cameraPos.x = glm::floor(cameraPos.x)+wallSize;
    if(grid[floor(cameraPos.z)][floor(cameraPos.x)].wallRight && distance(cameraPos.x,glm::floor(cameraPos.x)+1.0f) <= wallSize)
        cameraPos.x = glm::floor(cameraPos.x)+1.0f-wallSize;
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

float distance(float x1, float x2)
{
    return glm::abs(x1-x2);
}