#ifndef HMI_GRAPHICS_H
#define HMI_GRAPHICS_H

#include <iostream>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <chrono>
#include "Node.h"
#include "Color.h"
#include "Game.h"
#include "stb_image.h"
#include <cctype>

extern "C" {
//#include <bcm_host.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <X11/Xatom.h>
#include <assert.h>
}
using namespace std;

#define CHEAT_MODE true

#define FOV_Y 45.0f
#define LOOKAT_Z 17.5


#define NUM_VERTICES 180 // define the number of vertices in the circle

#define ANIMATION_SPEED 3.5
#define ANIMATION_THRESHOLD 0.5

#define MAX_INTERSECTION_TIMOUT 15

#define DEFAULT_RADIUS 0.5
#define DEFAULT_START_POINT glm::vec3(0.0f,-5.5625f,0.0f)

#define check() assert(glGetError() == 0)

class Graphics {
private:
    // EGL handles
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    // Screen size for glViewport
    EGLint screenHeight;
    EGLint screenWidth;

    // MVP matrix
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;

    int _width = 800;
    int _height = 600;

    // shader and program handles
    GLuint program;

    // attribute and uniform locations
    GLuint aPosition;
    GLuint aProjection;
    GLuint aView;
    GLuint aModel;
    GLuint uColor;
    GLuint aTexCoord;
    GLuint texUniform;
    GLuint texUniform2;
    GLuint texturePlayfield;
    GLuint textureScene;
    GLuint textureCircleRed;
    GLuint textureCircle2;

    map<string,GLuint> circleTextures;

    // raspi4 globals
    Display *_xDisplay;
    Window _xWindow;

    // node screen positions
    map<string, pair<float, float>> nodePositions;

    // intersected nodes
    map<string, float> intersectedNodeNames;
    string nodeToAdd;

    // Game instance
    Game game;

    // For animations
    glm::vec3 currentAnimationDir;
    glm::vec3 currentAnimationPos;
    bool animationInProgress = false;
    vector<glm::vec3> animationPoints;
    std::chrono::time_point<std::chrono::system_clock> t_start = std::chrono::system_clock::now();

    //Mouse coordinates
    float mouse_posX = 0;
    float mouse_posY = 0;

    glm::vec3 intersectionPoint = glm::vec3(0);
    glm::vec3 newPoint = glm::vec3(0);
    glm::vec3 startingPoint = glm::vec3(0.0f,-5.5625f,0.0f);
    glm::vec3 endPoint = glm::vec3(0.0f, 10.0f, 0.0f);

    glm::vec3 bottomL = glm::vec3(-4.01999998f,-4.7750001, 0.0f);
    glm::vec3 bottomR = glm::vec3(4.01999998f,-4.7750001, 0.0f);
    glm::vec3 topR = glm::vec3(4.01999998f,6.7750001f, 0.0f);
    glm::vec3 topL = glm::vec3(-4.01999998f,6.7750001f, 0.0f);

    float spacingX = DEFAULT_RADIUS * 2 + 0.005;
    float spacingY = 0.875;

    float diffX = -3.5f * spacingX;
    float diffY = 6 * spacingY;

    float offsetY = 1;

    string lastHit = "";
    int intersectionTimout = 0;
    vector<GLfloat> lines = {};
    glm::vec3 lineColor = glm::vec3(255.0, 0.0, 0.0);

    glm::vec3 circleIntersectionPoint = glm::vec3(1);

    void showCompilerLog(GLint shader);
    void showLinkerLog(GLint prog);
    void resize();
    void createXWindow();
    void initOGL();
    void print_shader_info_log(GLuint shader);
    GLuint load_shader(GLenum type, const string &path);
    void initShaders();
    void initNodePositions();
    void setupViewport();
    void drawSquare(GLfloat squareData[]);
    void drawCircle(GLfloat centerX = 0.0, GLfloat centerY = 0.0, GLfloat radius = DEFAULT_RADIUS);
    void drawCircleByName(string name, Color color, glm::vec2 offsetPos);
    void drawLine();
    void calculateNewPosition(bool showLines = false);
    glm::vec3 get_line_intersection(glm::vec3 bottomBorder, glm::vec3 topBorder);
    glm::vec3 calculateReflectionDir(glm::vec3 bottom, glm::vec3 top);
    string circleIntersection();
    glm::vec3 proj(glm::vec3 a, glm::vec3 b);
    float hypot2(glm::vec3 a, glm::vec3 b);
    float calcDistanceFromCircleToEndStart(float x, float y);

    void loadTexture(string filePath, GLuint& texture);
    void initCircleTextures();
    void circleTexture();

    void findFinalPosition(string hitNode);

    pair<float, float> screenToWorld(int screenPosX, int screenPosY);
    void resetState();
    void startAnimation();
    void stopAnimation();
    void drawAnimatedShootCircle(Color color, double t_frame);

public:
    Graphics();

    void draw();
    void handleXEvents();
};


#endif //HMI_GRAPHICS_H
