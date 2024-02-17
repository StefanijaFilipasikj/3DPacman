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
#include <AL/al.h>
#include <AL/alc.h>
#include <cstdio>
#include <cstdlib>
#include FT_FREETYPE_H

const std::string program_name = ("3D PACMAN");

// declare functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
float distance(float x1, float x2);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void loadGhost(Ghost &ghost, Model &ghostModel, Model &scaredModel, Shader &shader);
void loadGhosts(Ghost &blinkyGhost, Ghost &pinkyGhost, Ghost &inkyGhost, Ghost &clydeGhost, Model &blinkyModel, Model &pinkyModel, Model &inkyModel, Model &clydeModel, Model &scaredModel, Shader &shader);
void loadWalls(glm::mat4 &model, Shader &shader, Model &wall);
void loadCoinsAndPowerups(glm::mat4 &model, Shader &shader, Model &coin, Model &powerup);
void checkGhostCollision(Ghost &ghost, const glm::vec3 &resetPosition);
void ghostCollision(Ghost &blinkyGhost, Ghost &pinkyGhost, Ghost &inkyGhost, Ghost &clydeGhost);
void pickupsCollision(ALuint coinSound, ALuint powerupSound);
void ghostScared(Ghost &blinkyGhost, Ghost &pinkyGhost, Ghost &inkyGhost, Ghost &clydeGhost);
float getDistance(glm::vec3 pos1, glm::vec3 pos2);
void renderText(Shader shaderProgram,std::string text, std::string text_position_x, float y, float scale, glm::vec3 color);
ALboolean loadWavFile(const char *path, ALenum *format, ALvoid **data, ALsizei *size, ALsizei *frequency);
ALuint loadSound(const char* filePath, ALboolean loop);
void startGame();

// screen resolution settings
const unsigned int SCR_WIDTH = 985;
const unsigned int SCR_HEIGHT = 700;

// position camera in the centre of the maze
glm::vec3 cameraPos   = glm::vec3(cols/2+0.5f, 0.5f,  rows/2+0.5f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::mat4 view = glm::mat4(1.0f);

// game classes
Ghost blinkyGhost, pinkyGhost, inkyGhost, clydeGhost; // red, pink, blue, orange ghost
Maze mazeClass;

static bool GAMEOVER = false;
bool gameOverSoundPlayed = false;
int points = 0;
float timer = 0;
float wallSize = 0.3f;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

struct Character {
    unsigned int TextureID; // id handle of the glyph texture
    glm::ivec2   Size;      // size of glyph
    glm::ivec2   Bearing;   // offset from baseline to left/top of glyph
    unsigned int Advance;   // horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters; // used for text rendering

unsigned int VBO, VAO;

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

    // load vertex and fragment shaders
    // --------------------------------
    Shader textShader("../../../shaders/text.vert","../../../shaders/text.frag");
    Shader minimapShader("../../../shaders/minimap.vert", "../../../shaders/minimap.frag");
    Shader modelShader("../../../shaders/model_loading.vert", "../../../shaders/model_loading.frag");
    Shader lightingModelShader("../../../shaders/lighting.vert", "../../../shaders/lighting.frag");

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;

    // all functions return a value different from 0 whenever an error occurred
    // initialize FreeType library
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
            // load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }

            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // store character for later use
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
    // destroy FreeType once finished
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

    // ui text
    // -----------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // minimap
    // -----------
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

    // frame buffer for minimap
    GLuint fbo;
    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

    // texture for minimap
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

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    // hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // load blender models
    // -------------------
    Model floor("../../../blender-objects/colliders/pacman_floor.obj");
    Model wall("../../../blender-objects/colliders/pacman_wall.obj");
    Model coin("../../../blender-objects/pickups/pacman_coin.obj");
    Model powerup("../../../blender-objects/pickups/pacman_powerup.obj");

    Model pacman("../../../blender-objects/ghosts/player.obj");
    Model blinky("../../../blender-objects/ghosts/pacman_ghost_red.obj");
    Model pinky("../../../blender-objects/ghosts/pacman_ghost_pink.obj");
    Model inky("../../../blender-objects/ghosts/pacman_ghost_blue.obj");
    Model clyde("../../../blender-objects/ghosts/pacman_ghost_orange.obj");
    Model scaredGhost("../../../blender-objects/ghosts/pacman_ghost_scared.obj");

    // OpenAL initialization
    // ---------------------
    ALCdevice *device = alcOpenDevice(NULL);
    ALCcontext *context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    // load and play game theme (looped)
    ALuint gameThemeSource = loadSound("../../../sounds/sound-game-theme.wav", AL_TRUE);
    alSourcePlay(gameThemeSource);

    // load other sounds for later: coin, powerup, win, loss
    ALuint coinSound = loadSound("../../../sounds/sound-effect-consume-coin.wav", AL_FALSE);
    ALuint powerupSound = loadSound("../../../sounds/sound-effect-consume-powerup.wav", AL_FALSE);
    ALuint gameOverSoundWin = loadSound("../../../sounds/sound-effect-game-over-win.wav", AL_FALSE);
    ALuint gameOverSoundLoss = loadSound("../../../sounds/sound-effect-game-over-loss.wav", AL_FALSE);

    startGame();

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // FIRST RENDER
        // ------------
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);

        // camera for minimap
        glm::vec3 mCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 mCameraPos = glm::vec3(5.0f, 13.0f, 5.0f);
        if(SCR_WIDTH < SCR_HEIGHT)
            mCameraPos = glm::vec3(5.0f, 16.0f, 5.0f); // a little more zoomed out if portrait

        // view and projection for minimap
        glm::mat4 view2 = glm::lookAt(mCameraPos, mCameraFront + mCameraPos, cameraUp);
        glm::mat4 projection2 = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        projection2 = glm::rotate(projection2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // load floor for minimap
        lightingModelShader.use();
        lightingModelShader.setMat4("model", model);
        lightingModelShader.setMat4("view", view2);
        lightingModelShader.setMat4("projection", projection2);
        floor.Draw(lightingModelShader);

        // load models for minimap
        modelShader.use();
        modelShader.setMat4("model", model);
        modelShader.setMat4("view", view2);
        modelShader.setMat4("projection", projection2);

        // load player (pacman) behind camera
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cameraPos.x, 1.0f, cameraPos.z));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        modelShader.setMat4("model", model);
        pacman.Draw(modelShader);

        // load walls, ghosts, coins & powerups
        loadWalls(model, modelShader, wall);
        loadGhosts(blinkyGhost, pinkyGhost, inkyGhost, clydeGhost,blinky, pinky, inky, clyde, scaredGhost, modelShader);
        loadCoinsAndPowerups(model, modelShader, coin, powerup);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

        // SECOND RENDER
        // -------------
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        model = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
        projection = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // lighting
        lightingModelShader.use();
        lightingModelShader.setMat4("model", model);
        lightingModelShader.setMat4("view", view);
        lightingModelShader.setMat4("projection", projection);
        lightingModelShader.setVec3("viewPos", cameraPos);
        lightingModelShader.setFloat("material.shininess", 32.0f);

        for(int i=0; i < rows; i++){
            for(int j=0;j< cols;j++){
                if(maze[i][j].hasCoin){
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

        // spotlight
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

        // load walls
        loadWalls(model, lightingModelShader, wall);

        // load ghosts
        modelShader.use();
        modelShader.setMat4("model", model);
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", projection);
        if(!GAMEOVER){
            blinkyGhost.move(deltaTime, std::floor(cameraPos.x) + std::floor(cameraPos.z) * cols);
            pinkyGhost.move(deltaTime, std::floor(cameraPos.x) + std::floor(cameraPos.z) * cols);
            inkyGhost.move(deltaTime, std::floor(cameraPos.x) + std::floor(cameraPos.z) * cols);
            clydeGhost.move(deltaTime, std::floor(cameraPos.x) + std::floor(cameraPos.z) * cols);
        }
        loadGhosts(blinkyGhost, pinkyGhost, inkyGhost, clydeGhost, blinky, pinky, inky, clyde, scaredGhost, modelShader);

        // load coins
        loadCoinsAndPowerups(model, modelShader, coin, powerup);

        ghostScared(blinkyGhost, pinkyGhost, inkyGhost, clydeGhost);
        pickupsCollision(coinSound, powerupSound);
        ghostCollision(blinkyGhost, pinkyGhost, inkyGhost, clydeGhost);

        // render text
        if(!GAMEOVER){
            renderText(textShader, "Points: "+ to_string(points), "left", SCR_HEIGHT-50, 1.0f, glm::vec3(1.0, 1.0f, 1.0f));
        }else{
            if(points == 1000){ // all coins and powerups are collected, you win
                renderText(textShader, "CONGRATS", "center", int(SCR_HEIGHT/1.8), 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
                renderText(textShader, "YOU WON!", "center", int(SCR_HEIGHT/2.5), 1.5f, glm::vec3(1.0, 1.0f, 1.0f));

                if(!gameOverSoundPlayed){
                    alSourcePlay(gameOverSoundWin);
                    gameOverSoundPlayed = true;
                }
            }else{ // a ghost is at the same position as you, you loose
                renderText(textShader, "GAME OVER", "center", int(SCR_HEIGHT/1.8), 2.0f, glm::vec3(1.0, 1.0f, 1.0f));
                renderText(textShader, "PRESS ENTER TO RESTART", "center", int(SCR_HEIGHT/2.5), 1.5f, glm::vec3(1.0, 1.0f, 1.0f));

                if(!gameOverSoundPlayed){
                    alSourcePlay(gameOverSoundLoss);
                    gameOverSoundPlayed = true;
                }
            }
        }

        // minimap
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, img);
        // if screen is in portrait, offset y-axis to get a square appearance of minimap, if it's landscape offset x-axis
        minimapShader.use();
        if(SCR_WIDTH < SCR_HEIGHT){
            float offset = (1.0 / ((float)SCR_HEIGHT/(float)SCR_WIDTH) - 1.0) / 2.0;
            minimapShader.setFloat("offset_x",  0.0f);
            minimapShader.setFloat("offset_y",  offset);
        }else{
            float offset = (1.0 / ((float)SCR_WIDTH/(float)SCR_HEIGHT) - 1.0) / 2.0;
            minimapShader.setFloat("offset_x",  offset);
            minimapShader.setFloat("offset_y",  0.0f);
        }

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

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

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void startGame(){
    mazeClass = Maze();
    mazeClass.generateMaze();
    GAMEOVER = false;
    points = 0;

    blinkyGhost = Ghost(glm::vec3(0.0f,0.0f,0.0f));
    pinkyGhost = Ghost(glm::vec3(cols-1,0.0f,rows-1));
    inkyGhost = Ghost(glm::vec3(cols-1,0.0f,0.0f));
    clydeGhost = Ghost(glm::vec3(0.0f,0.0f,rows-1));

    cameraPos   = glm::vec3(cols/2+0.5f, 0.5f,  rows/2+0.5f);
}

void ghostScared(Ghost &blinkyGhost, Ghost &pinkyGhost, Ghost &inkyGhost, Ghost &clydeGhost) {
    std::vector<std::reference_wrapper<Ghost>> ghosts = {std::ref(blinkyGhost), std::ref(pinkyGhost), std::ref(inkyGhost), std::ref(clydeGhost)};

    // if the timer is greater than 0, it indicated that the ghosts are scared
    if(timer > 0){
        timer-=deltaTime;
        for (Ghost &ghost : ghosts)
            ghost.isScared = true;
    }
    if(timer < 0){
        timer = 0;
        for (Ghost &ghost : ghosts)
            ghost.isScared = false;
    }
}

void pickupsCollision(ALuint coinSound, ALuint powerupSound) {
    // if the current maze cell has a coin add 10 points and play sound
    if(maze[floor(cameraPos.z)][floor(cameraPos.x)].hasCoin){
        maze[floor(cameraPos.z)][floor(cameraPos.x)].hasCoin = false;
        points += 10;
        alSourcePlay(coinSound);
    }
    // if the current maze cell has a powerup add 10 points, start the timer (for scared ghosts) and play sound
    if(maze[floor(cameraPos.z)][floor(cameraPos.x)].hasPowerup){
        maze[floor(cameraPos.z)][floor(cameraPos.x)].hasPowerup = false;
        points += 10;
        timer = 5;
        alSourcePlay(powerupSound);
    }
    // if 1000 points are collected, the game is over, the player won
    if(points == 1000){
        GAMEOVER = true;
    }
}

void checkGhostCollision(Ghost &ghost, const glm::vec3 &resetPosition) {
    // check if a ghost is at the same spot as pacman
    if (getDistance(cameraPos, ghost.position) <= 0.5f) {
        if (ghost.isScared) {
            ghost = Ghost(resetPosition);
        } else {
            GAMEOVER = true; // if not scared, the game is over, the player lost
        }
    }
}

void ghostCollision(Ghost &blinkyGhost, Ghost &pinkyGhost, Ghost &inkyGhost, Ghost &clydeGhost) {
    checkGhostCollision(blinkyGhost, glm::vec3(0.0f, 0.0f, 0.0f));
    checkGhostCollision(pinkyGhost, glm::vec3(cols - 1, 0.0f, rows - 1));
    checkGhostCollision(inkyGhost, glm::vec3(cols - 1, 0.0f, 0.0f));
    checkGhostCollision(clydeGhost, glm::vec3(0.0f, 0.0f, rows - 1));
}

float getDistance(glm::vec3 pos1, glm::vec3 pos2){
    // calculate Euclidean distance between two points
    return sqrt(pow(pos1.x-(pos2.x + 0.5f),2) + pow(pos1.z-(pos2.z + 0.5f),2));
}

void loadCoinsAndPowerups(glm::mat4 &model, Shader &shader, Model &coin, Model &powerup) {
   // iterate through maze cells, check if the current cell has a coin or powerup, position, scale and draw it
    for(int i=0; i < rows; i++){
        for(int j=0;j<cols;j++){
            if(maze[i][j].hasCoin){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3( j + 0.5f, 0.1f, i + 0.5f));
                model = glm::scale(model, glm::vec3( 0.08f, 0.08f, 0.08f));
                shader.setMat4("model", model);
                coin.Draw(shader);
            }
            if(maze[i][j].hasPowerup){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3( j + 0.5f, 0.1f, i + 0.5f));
                model = glm::scale(model, glm::vec3( 0.08f, 0.08f, 0.08f));
                shader.setMat4("model", model);
                powerup.Draw(shader);
            }
        }
    }
}

void loadGhost(Ghost &ghost, Model &ghostModel, Model &scaredModel, Shader &shader) {

    //scale, translate and rotate the model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3( ghost.position.x  + 0.5f, 0.3f, ghost.position.z + 0.5f));
    model = glm::rotate(model, glm::radians(ghost.rotation), glm::vec3(0, 1.0f, 0));
    model = glm::scale(model, glm::vec3( 0.2f, 0.2f, 0.2f));
    shader.setMat4("model", model);

    // if scared draw the scaredModel, otherwise draw the regular ghost model (blinky, pinky, inky or clyde)
    if (ghost.isScared)
        scaredModel.Draw(shader);
    else
        ghostModel.Draw(shader);
}

void loadGhosts(Ghost &blinkyGhost, Ghost &pinkyGhost, Ghost &inkyGhost, Ghost &clydeGhost,Model &blinkyModel,
                Model &pinkyModel, Model &inkyModel, Model &clydeModel, Model &scaredModel, Shader &shader) {
    loadGhost(blinkyGhost, blinkyModel, scaredModel, shader);
    loadGhost(pinkyGhost, pinkyModel, scaredModel, shader);
    loadGhost(inkyGhost, inkyModel, scaredModel, shader);
    loadGhost(clydeGhost, clydeModel, scaredModel, shader);
}

void loadWalls(glm::mat4 &model, Shader &shader, Model &wall) {
    float scale = 0.132f;

    // iterate through each cell in the maze
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            // draw left wall for each cell if wall exists
            if(maze[i][j].wallLeft){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(j, 0.05f, (float)(i)+0.5f));
                model = glm::scale(model, glm::vec3( scale, scale, scale));
                shader.setMat4("model", model);
                wall.Draw(shader);
            }
            // draw top wall for each cell if wall exists
            if(maze[i][j].wallUp){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)(j)+0.5f, 0.05f, i));
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0));
                model = glm::scale(model, glm::vec3( scale, scale, scale));
                shader.setMat4("model", model);
                wall.Draw(shader);
            }
        }
    }
    // for each cell in the last column, draw right wall
    for(int j=0;j<rows;j++){
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(rows, 0.05f, (float)(j)+0.5f));
        model = glm::scale(model, glm::vec3( scale, scale, scale));
        shader.setMat4("model", model);
        wall.Draw(shader);
    }
    // for each cell in the last row, draw bottom wall
    for(int j=0;j<cols;j++){
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((float)(j)+0.5f, 0.05f, rows));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1.0f, 0));
        model = glm::scale(model, glm::vec3( scale, scale, scale));
        shader.setMat4("model", model);
        wall.Draw(shader);
    }
}

void renderText(Shader shaderProgram,std::string text, std::string text_position_x, float y, float scale, glm::vec3 color)
{
    shaderProgram.use();
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    std::string::const_iterator c;
    scale = scale * (SCR_WIDTH / 1000.0f);
    float x;

    if(text_position_x == "center")
    {
        float text_width = 0.0f;
        for (char c : text){
            Character ch = Characters[c];
            text_width += (ch.Advance >> 6) * scale;
        }
        // starting point of text should be half of the screen width minus half of the text width for it to be centered
        x = SCR_WIDTH/2.0f - text_width/2.0f;

    }else{ //text_position_x == left
        x = 1.0f;
    }

    for (char c : text){
        Character ch = Characters[c];

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
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // advance cursors for next glyph
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ALboolean loadWavFile(const char *path, ALenum *format, ALvoid **data, ALsizei *size, ALsizei *frequency) {
    // open the file in binary read mode ("rb").
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return AL_FALSE;
    }

    // determine the size of a file
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // allocate dynamic memory to store the contents of the file
    *data = malloc(*size);
    if (!*data) {
        fprintf(stderr, "Failed to allocate memory for file data\n");
        fclose(file);
        return AL_FALSE;
    }

    // read the data from the file into the previously allocated memory block
    fread(*data, 1, *size, file);
    fclose(file);

    // assume a mono, 16-bit PCM WAV file for simplicity
    *format = AL_FORMAT_MONO16;
    *frequency = 44100;

    return AL_TRUE;
}

ALuint loadSound(const char* filePath, ALboolean loop)
{
    ALuint soundBuffer, soundSource;
    ALsizei soundSize, soundFrequency;
    ALenum soundFormat;
    ALvoid* soundData;

    // try loading the file using the loadWavFile function
    if (!loadWavFile(filePath, &soundFormat, &soundData, &soundSize, &soundFrequency)) {
        fprintf(stderr, "Failed to load sound WAV file: %s\n", filePath);
        return 0;
    }

    // generate sound buffer
    alGenBuffers(1, &soundBuffer);
    alBufferData(soundBuffer, soundFormat, soundData, soundSize, soundFrequency);

    // generate sound source
    alGenSources(1, &soundSource);
    alSourcei(soundSource, AL_BUFFER, soundBuffer);

    // set looping property
    alSourcei(soundSource, AL_LOOPING, loop);

    // clean up loaded data
    free(soundData);

    return soundSource;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    // camera movement speed scaled by deltaTime for smooth movement
    float cameraSpeed = 4.0f * deltaTime;

    glm::vec3 camera = glm::vec3(1.0f, 0.0f,  1.0f);

    // close the application when ESC key is pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // if game is not over, move camera based on key presses
    if(!GAMEOVER){
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront * camera;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront * camera;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront * camera, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront * camera, cameraUp)) * cameraSpeed;
    }else{
        // if game is over and enter is pressed, restart game
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            startGame();
            gameOverSoundPlayed = false;
        }
    }

    // wall collisions to prevent the camera from moving through walls
    if(maze[floor(cameraPos.z)][floor(cameraPos.x)].wallUp && distance(cameraPos.z,glm::floor(cameraPos.z)) <= wallSize)
        cameraPos.z = glm::floor(cameraPos.z)+wallSize;
    if(maze[floor(cameraPos.z)][floor(cameraPos.x)].wallDown && distance(cameraPos.z,glm::floor(cameraPos.z)+1.0f) <= wallSize)
        cameraPos.z = glm::floor(cameraPos.z)+1.0f-wallSize;
    if(maze[floor(cameraPos.z)][floor(cameraPos.x)].wallLeft && distance(cameraPos.x,glm::floor(cameraPos.x)) <= wallSize)
        cameraPos.x = glm::floor(cameraPos.x)+wallSize;
    if(maze[floor(cameraPos.z)][floor(cameraPos.x)].wallRight && distance(cameraPos.x,glm::floor(cameraPos.x)+1.0f) <= wallSize)
        cameraPos.x = glm::floor(cameraPos.x)+1.0f-wallSize;
}

float distance(float x1, float x2)
{
    return glm::abs(x1-x2);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
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
