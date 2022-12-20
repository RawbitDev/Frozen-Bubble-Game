#include "Graphics.h"

Graphics::Graphics() {
    game.nextLevel();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    cout << "Initializing OpenGL ES..." << endl;
    initOGL();

    cout << "Initializing shaders..." << endl;
    initShaders();

    cout << "Setting up textures..." << endl;
    string playfield = "../textures/playfield.png";
    string scene = "../textures/scene.png";
    loadTexture(scene, textureScene);
    loadTexture(playfield, texturePlayfield);
    initCircleTextures();

    cout << "Initializing node screen positions..." << endl;
    initNodePositions();

    cout << "Setting up viewport..." << endl;
    setupViewport();

}

void Graphics::initCircleTextures(){

    for(auto color: game.getCurrentLevel().getCurrentColors()){
        GLuint texture;
        string path = "../textures/"+ color.first+".png";
        loadTexture(path,texture);
        circleTextures.insert(make_pair(color.first,texture));
    }

}

void Graphics::showCompilerLog(GLint shader) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof log, NULL, log);
    cout << "Shader (#" << shader << ") compilation:\n" << log << endl;
}

void Graphics::showLinkerLog(GLint prog) {
    char log[1024];
    glGetProgramInfoLog(program, sizeof log, NULL, log);
    cout << "Shader (#" << program << ") linking:\n" << log << endl;
}

void Graphics::resize() {
    // Raspberry Pi 4: query window size from X11
    XWindowAttributes gwa;
    XGetWindowAttributes(_xDisplay, _xWindow, &gwa);
    _width = gwa.width;
    _height = gwa.height;
    glViewport(0, 0, _width, _height);
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    projection = glm::perspective(glm::radians(FOV_Y), (float) _width / (float) _height, 0.1f, 100.0f);
}

pair<float, float> Graphics::screenToWorld(int screenPosX, int screenPosY) {
    float aspect = (float) _width / (float) _height;
    float radFovY = FOV_Y / 180 * M_PI;
    float radFovX = 2 * atan(tan(radFovY / 2) * aspect);
    float worldWidth = static_cast<float>(tan(radFovX / 2) * LOOKAT_Z * 2);

    int normalizedScreenX = (screenPosX - _width / 2);
    int normalizedScreenY = (screenPosY - _height / 2);

    float worldX = (float) normalizedScreenX / (float) _width * worldWidth;
    float worldY = (float) -normalizedScreenY / (float) _width * worldWidth;
    return make_pair(worldX, worldY);
}


glm::vec3 Graphics::get_line_intersection(glm::vec3 bottomBorder, glm::vec3 topBorder) {
    float s1_x, s1_y, s2_x, s2_y;

    s1_x = topBorder.x - bottomBorder.x;
    s1_y = topBorder.y - bottomBorder.y;
    s2_x = endPoint.x - startingPoint.x;
    s2_y = endPoint.y - startingPoint.y;
    float s, t;
    s = (-s1_y * (bottomBorder.x - startingPoint.x) + s1_x * (bottomBorder.y - startingPoint.y)) /
        (-s2_x * s1_y + s1_x * s2_y);

    t = (s2_x * (bottomBorder.y - startingPoint.y) - s2_y * (bottomBorder.x - startingPoint.x)) /
        (-s2_x * s1_y + s1_x * s2_y);

    float i_x = 0;
    float i_y = 0;

    i_x = bottomBorder.x + (t * s1_x);
    i_y = bottomBorder.y + (t * s1_y);

    return glm::vec3(i_x, i_y, 0.0f);
}

glm::vec3 Graphics::calculateReflectionDir(glm::vec3 bottom, glm::vec3 top) {
    intersectionPoint = get_line_intersection(bottom, top);
    glm::vec3 R = bottom - top;
    glm::vec3 N = glm::vec3(R.y, -R.x, 0.0f);
    N = glm::normalize(N);
    glm::vec3 dir = endPoint - startingPoint;
    auto test = glm::dot(R, N);
    glm::vec3 reflection = dir - 2.0f * dot(dir, N) * N;
    reflection = glm::normalize(reflection);

    return reflection;
}

glm::vec3 Graphics::proj(glm::vec3 a, glm::vec3 b) {
    const float k = glm::dot(a, b) / glm::dot(b, b);
    return glm::vec3(k * b.x, k * b.y, 0.0f);
}

float Graphics::hypot2(glm::vec3 a, glm::vec3 b) {
    return glm::dot(a - b, a - b);
}

float Graphics::calcDistanceFromCircleToEndStart(float x, float y) {
    glm::vec3 circle = glm::vec3(x, y, 0.0f);
    glm::vec3 endToCircle = circle - endPoint;
    glm::vec3 endToStart = startingPoint - endPoint;

    const glm::vec3 D = endPoint + proj(endToCircle, endToStart);
    const glm::vec3 AD = D - endPoint;
    const float k = abs(endToStart.x) > abs(endToStart.y) ? AD.x / endToStart.x : AD.y / endToStart.y;

    if (k <= 0.0f) {
        return ::sqrt(hypot2(circle, endPoint));
    } else if (k >= 1.0f) {
        return ::sqrt(hypot2(circle, startingPoint));
    }
    return ::sqrt(hypot2(circle, D));
}

void Graphics::findFinalPosition(string hitNode) {
    float smallestDistance = 100000;
    string _nodeName = "Name";
    auto nodeNeigh = game.getFullGraphLevel().getGraph().getNeighbors(hitNode);
    for (const auto &nodeName: nodeNeigh) {
        auto res = intersectedNodeNames.find(nodeName);
        if (res != intersectedNodeNames.end()) {
            if (res->second <= smallestDistance) {
                _nodeName = res->first;
                smallestDistance = res->second;
            }
        }
    }
    nodeToAdd = _nodeName;
}

string Graphics::circleIntersection() {
    intersectedNodeNames.clear();
    string nodeA = "";
    float smallestDistance = 100000;
    string _nodeName = "Name";
    for (const auto &n: nodePositions) {
        string nodeName = n.first;
        if (nodeName != "QUEUE_0" && nodeName != "QUEUE_1") {

            pair<float, float> node = n.second;
            glm::vec3 circlePos = glm::vec3(node.first, node.second, 0.0f);

            float distance = calcDistanceFromCircleToEndStart(circlePos.x, circlePos.y);

            // 1.75 das kein laserstrahl sondern Kugel angenähert wird
            if (distance <= DEFAULT_RADIUS * 1.75) {
                float distanceToCircle = glm::length(circlePos - startingPoint);
                intersectedNodeNames.insert(make_pair(nodeName, distanceToCircle));
                if (nodeName[0] == 'A' && !game.getCurrentLevel().getGraph().getNode(nodeName)) {
                    nodeA = nodeName;
                }
                if (game.getCurrentLevel().getGraph().getNode(nodeName)) {
                    if (distanceToCircle < smallestDistance) {
                        smallestDistance = distanceToCircle;
                        _nodeName = nodeName;
                    }
                }
            }
        }
    }
    // std::cout << "Intersection -> " << _nodeName << " (Distance: " << smallestDistance << ")" << std::endl;
    if (_nodeName == "Name") {
        if (!nodeA.empty()) {
            nodeToAdd = nodeA;
        }
    }
    lastHit = _nodeName;
    return _nodeName;
}

void Graphics::calculateNewPosition(bool showLines) {
    glm::vec3 reflectionDir = glm::vec3(1);
    glm::vec3 borderDir = glm::vec3(1);
    if (intersectionTimout++ > MAX_INTERSECTION_TIMOUT) return;

    if (endPoint.x < 0) {
        reflectionDir = calculateReflectionDir(bottomL, topL);
        newPoint = intersectionPoint + reflectionDir * 15.0f;
        endPoint = intersectionPoint;

        if (showLines) {
            lines.insert(lines.end(), {
                    startingPoint.x, startingPoint.y, 0.0f,
                    endPoint.x, endPoint.y, 0.0f
            });
        }

        if (circleIntersection() == "Name") {
            if (endPoint.y <= topL.y) { // Ignore animation points that are out of bounce
                animationPoints.push_back(endPoint);
            }
            startingPoint = endPoint;
            endPoint = newPoint;
            calculateNewPosition(showLines);
        } else {
            if (nodeToAdd.empty()) findFinalPosition(lastHit);
        }

    } else if (endPoint.x > 0) {
        reflectionDir = calculateReflectionDir(bottomR, topR);
        newPoint = intersectionPoint + reflectionDir * 15.0f;
        endPoint = intersectionPoint;

        if (showLines) {
            lines.insert(lines.end(), {
                    startingPoint.x, startingPoint.y, 0.0f,
                    endPoint.x, endPoint.y, 0.0f
            });
        }

        if (circleIntersection() == "Name") {
            if (endPoint.y <= topL.y) {  // Ignore animation points that are out of bounce
                animationPoints.push_back(endPoint);
            }
            startingPoint = endPoint;
            endPoint = newPoint;
            calculateNewPosition(showLines);
        } else {
            if (nodeToAdd.empty()) findFinalPosition(lastHit);
        }
    }
    //std::cout << "New Intersect-> X: " << intersectionPoint.x << " Y: " << intersectionPoint.y << std::endl;
}


void Graphics::resetState() {
    lastHit = "";
    nodeToAdd = "";
    lines.clear();
    intersectionTimout = 0;
    animationPoints.clear();
    startingPoint = DEFAULT_START_POINT;
    lineColor = glm::vec3(255.0, 0.0, 0.0);
    endPoint = glm::vec3(mouse_posX, mouse_posY, 0.0f);
}

void Graphics::handleXEvents() {
    if (animationInProgress) return;
    bool quit = false;

    // under Raspberry Pi 4 we can process keys from X11
    while (XPending(_xDisplay)) {   // check for events from the x-server
        XEvent xev;
        XNextEvent(_xDisplay, &xev);

        if (xev.type == MotionNotify) {
            // std::cout << "X: " << xev.xmotion.x  << " Y: " << xev.xmotion.y  << std::endl;
            pair<float, float> mousePos = screenToWorld(xev.xmotion.x, xev.xmotion.y);
            mouse_posX = min(max(mousePos.first, topL.x), topR.x);
            mouse_posY = min(max(mousePos.second, bottomL.y), topL.y);
            endPoint = glm::vec3(mouse_posX, mouse_posY, 0.0f);

            if (CHEAT_MODE) {
                resetState();
                calculateNewPosition(true);
            }
            // std::cout << "X: " << mouse_posX << " Y: " << mouse_posY << std::endl;
        }
        if (xev.type == KeyPress) {
            switch (xev.xkey.keycode) {
                case 9://ESC
                case 24://Q
                    exit(0);
                    break;
            }
        }
        if (xev.type == ButtonPress) {
            switch (xev.xbutton.button) {
                case 1:// left mouse button or touch screen
                    lineColor = glm::vec3(0.0, 255.0, 0.0);
                    calculateNewPosition(true);
                    if (intersectionTimout <= MAX_INTERSECTION_TIMOUT || !nodeToAdd.empty()) {
                        //std::cout << "HIT -> " << lastHit << std::endl;
                        //std::cout << "new Node -> " << nodeToAdd[0] <<  (int)nodeToAdd[1]-48 << std::endl;
                        startAnimation();
                    }
                    break;
            }
        }
        if (xev.type == ButtonRelease) {
            //game.shoot();
            switch (xev.xbutton.button) {
                case 1:
                    resetState();
                    if (CHEAT_MODE) {
                        lineColor = glm::vec3(255.0, 0.0, 0.0);
                        calculateNewPosition(true);
                    }
                    break;
            }

        }
        if (xev.type == Expose) {}
        if (xev.type == PropertyNotify) {
            resize();
            break;
        }
    }
}

void Graphics::createXWindow() {

    _xDisplay = XOpenDisplay(NULL);
    if (NULL == _xDisplay) {
        throw runtime_error("could not connect to X server");
    }

    Window root = DefaultRootWindow(_xDisplay);

    XSetWindowAttributes windowAttributes;
    windowAttributes.event_mask =
            PointerMotionMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | Button1MotionMask | ExposureMask |
            PropertyChangeMask;

    _xWindow = XCreateWindow(_xDisplay, root,
                             0, 0, _width, _height,
                             0, CopyFromParent, InputOutput, CopyFromParent,
                             CWEventMask, &windowAttributes);

// the code below follows
// https://wiki.maemo.org/SimpleGL_example
// it may or may not be needed, depending on platform and window manager.

    // override window placement by the window manager
    windowAttributes.override_redirect = False;
    XChangeWindowAttributes(_xDisplay, _xWindow, CWOverrideRedirect, &windowAttributes);

    // change to fullscreen on ubuntu
    Atom atom = XInternAtom(_xDisplay, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty(
            _xDisplay, _xWindow,
            XInternAtom(_xDisplay, "_NET_WM_STATE", True),
            XA_ATOM, 32, PropModeReplace,
            (unsigned char *) &atom, 1);

    // set focus management
    XWMHints hints;
    hints.input = True;             // get focus from wm without explicitly setting it
    hints.flags = InputHint;        // enable the input field above
    XSetWMHints(_xDisplay, _xWindow, &hints);

    // change window title
    XStoreName(_xDisplay, _xWindow, "OpenGL - Frozen Bubble");

    // map window -- causes it to be finally displayed
    XMapWindow(_xDisplay, _xWindow);

    // another means to communicate fullscreen to a window manager
    // also works on ubuntu
    Atom wm_state = XInternAtom(_xDisplay, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(_xDisplay, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));

    xev.type = ClientMessage;
    xev.xclient.window = _xWindow;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = fullscreen;
    XSendEvent(                // send an event mask to the X-server
            _xDisplay,
            DefaultRootWindow (_xDisplay),
            False,
            SubstructureNotifyMask,
            &xev);
}

/*
 * initOGL()
 *
 * sets up an OpenGL ES2 surface and clears the screen to hda_darkblue
 */

void Graphics::initOGL() {
    EGLBoolean result;
    EGLint num_config;

    static const EGLint attribute_list[] =
            {
                    EGL_RED_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_BLUE_SIZE, 8,
                    EGL_ALPHA_SIZE, 8,
                    EGL_NONE
            };

    static const EGLint context_attributes[] =
            {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
            };
    EGLConfig config;


    // get an EGL display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(display != EGL_NO_DISPLAY);
    check();

    // initialize the EGL display connection
    result = eglInitialize(display, NULL, NULL);
    assert(EGL_FALSE != result);
    check();

    // get an appropriate EGL frame buffer configuration
    result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);
    check();

    // get an appropriate EGL frame buffer configuration
    result = eglBindAPI(EGL_OPENGL_ES_API);
    assert(EGL_FALSE != result);
    check();

    // create an EGL rendering context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
    assert(context != EGL_NO_CONTEXT);
    check();

    createXWindow();
    // create an EGL window surface
    surface = eglCreateWindowSurface(display, config, _xWindow, NULL);
    assert(surface != EGL_NO_SURFACE);
    check();

    // connect the context to the surface
    result = eglMakeCurrent(display, surface, surface, context);
    assert(EGL_FALSE != result);
    check();

    // Set background color and clear buffers
    glClearColor(0.0627f, 0.1765f, 0.3608f, 1.0f); // some dark blue
    glClear(GL_COLOR_BUFFER_BIT);

    check();
}

void Graphics::print_shader_info_log(GLuint shader) {  // handle to the shader
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    if (length) {
        char *buffer = new char[length];
        glGetShaderInfoLog(shader, length, NULL, buffer);
        cout << "shader info: " << buffer << flush;
        delete[] buffer;

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) exit(1);
    }
}

GLuint Graphics::load_shader(GLenum type, const string &path) {
    string Code;
    ifstream ShaderFile;

    ShaderFile.exceptions(ifstream::badbit);
    try {
        ShaderFile.open(path);
        if (!ShaderFile.is_open())
            throw invalid_argument("Vertex shader file not found.");

        stringstream ShaderStream;
        ShaderStream << ShaderFile.rdbuf();

        ShaderFile.close();

        Code = ShaderStream.str();
    } catch (const exception &ex) {
        string errmsg;
        errmsg.append("Error: Shader files couldn't be read:\n");
        errmsg.append(ex.what());
        throw logic_error(errmsg.c_str());
    }
    const GLchar *ShaderCode = Code.c_str();
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &ShaderCode, NULL);
    glCompileShader(shader);

    print_shader_info_log(shader);
    return shader;
}

void Graphics::initShaders() {
    GLuint vertexShader = load_shader(GL_VERTEX_SHADER, "shader/vertex.glsl");         // load vertex shader
    GLuint fragmentShader = load_shader(GL_FRAGMENT_SHADER, "shader/fragment.glsl");   // load fragment shader

    /* check();
    showCompilerLog(vertexShader);
    check();
    showCompilerLog(fragmentShader); */

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    check();

    // showLinkerLog(program);

    aPosition = glGetAttribLocation(program, "Position");
    aProjection = glGetUniformLocation(program, "Projection");
    aView = glGetUniformLocation(program, "View");
    aModel = glGetUniformLocation(program, "Model");
    uColor = glGetUniformLocation(program, "Color");
    aTexCoord = glGetUniformLocation(program, "aTexCoord");
    texUniform = glGetUniformLocation(program,"tex");
    texUniform2 = glGetUniformLocation(program,"tex2");
}

void Graphics::initNodePositions() {
    nodePositions.insert(make_pair("QUEUE_0",
                                   make_pair((float) 3.5 * spacingX + diffX,
                                             (float) 13.5 * -spacingY + diffY + offsetY)));

    // std::cout << "X Position: " << (float)3.5 * spacingX + diffX << " Y Position: " << (float)13.5 * -spacingY + diffY + offsetY << std::endl;
    nodePositions.insert(make_pair("QUEUE_1",
                                   make_pair((float) 5 * spacingX + diffX,
                                             (float) 14 * -spacingY + diffY + offsetY)));
    for (int y = 0; y < 13; ++y) {
        if (y % 2 == 0) {
            for (int x = 0; x < 8; ++x) {
                auto res = nodePositions.insert(make_pair(static_cast<char>('A' + y) + to_string(x),
                                                          make_pair((float) x * spacingX + diffX,
                                                                    (float) y * -spacingY + diffY + offsetY)));
                if (!res.second) {
                    throw invalid_argument("Cannot add nodePosition!");
                }
            }
        } else {
            for (int x = 0; x < 7; ++x) {
                auto res = nodePositions.insert(make_pair(static_cast<char>('A' + y) + to_string(x),
                                                          make_pair((float) x * spacingX + diffX + spacingX / 2,
                                                                    (float) y * -spacingY + diffY + offsetY)));
                if (!res.second) {
                    throw invalid_argument("Cannot add nodePosition!");
                }
            }
        }
    }
}

void Graphics::setupViewport() {
/*
   // query size using a broadcom function, raspi only
   int32_t success = graphics_get_display_size(0, &screenWidth, &screenHeight);
   assert( success >= 0 );
   cout << "screen is " << screenWidth << "x" << screenHeight << endl;
*/

    // query surface size using EGL, more general
    eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth);
    eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight);

    // Camera matrix
    view = glm::lookAt(
            glm::vec3(0, 0, LOOKAT_Z), // Camera in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    model = glm::mat4(1.0f);

    glViewport(0, 0, screenWidth, screenHeight);
    check();
}

void Graphics::drawSquare(GLfloat squareData[]) {

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

// Upload the vertex data to the VBO
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(squareData), squareData, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

// Enable the vertex attribute arrays
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);  // Vertex positions
    glEnableVertexAttribArray(0);  // Vertex positions

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);  // Vertex positions

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    GLenum  error = glGetError();
    check();
}

void Graphics::drawCircle(GLfloat centerX, GLfloat centerY, GLfloat radius) {
    GLfloat vertices[NUM_VERTICES][4];  // Array to store vertices and texture coordinates

    int i  = 0;
    for ( float angle=0.0; angle<360.0; angle+=2.0){
        float radian = angle * (M_PI/180.0f);

       float xcos = (float)cos(radian);
       float ysin = (float)sin(radian);
        vertices[i][0] = xcos * DEFAULT_RADIUS  + centerX;
        vertices[i][1] = ysin * DEFAULT_RADIUS* (_width/_height) + centerY;
        vertices[i][2] = xcos * 0.5 + 0.5;
        vertices[i][3] = ysin * 0.5 + 0.5;
        ++i;
    }


    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

// Upload the vertex data to the VBO
    glBufferData(GL_ARRAY_BUFFER, 4*NUM_VERTICES*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

// Enable the vertex attribute arrays
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);  // Vertex positions
    glEnableVertexAttribArray(0);  // Vertex positions

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));  // Texture coordinates
    glEnableVertexAttribArray(1);  // Texture coordinates


    glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERTICES);
    GLenum  error = glGetError();
    check();
}

void Graphics::drawCircleByName(string name, Color color, glm::vec2 offsetPos = glm::vec2(0)) {
    auto res = nodePositions.find(name);
    if (res == nodePositions.end()) {
        throw invalid_argument("Cannot draw circle: Name '" + name + "' wasn't found!");
        return;
    }
    const float x = res->second.first + offsetPos.x;
    const float y = res->second.second + offsetPos.y;

    auto circliename = circleTextures.find(color.colorName);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,circliename->second);
    glUniform1i(texUniform,1);
    //glUniform4f(uColor, float(color.r) / 255.0f, float(color.g) / 255.0f, float(color.b) / 255.0f, color.a);
    drawCircle(x, y);
}

void Graphics::drawAnimatedShootCircle(Color color, double t_frame) {
    if (animationPoints.empty() || currentAnimationPos.y > topL.y) { // Break animation if out of bounce or no more points left
        stopAnimation();
        return;
    }

    currentAnimationPos.x = currentAnimationPos.x + currentAnimationDir.x * t_frame * ANIMATION_SPEED;
    currentAnimationPos.y = currentAnimationPos.y + currentAnimationDir.y * t_frame * ANIMATION_SPEED;

    auto endPos = animationPoints.front();
    if (currentAnimationPos.x > (endPos.x - ANIMATION_THRESHOLD) &&
        currentAnimationPos.x<(endPos.x + ANIMATION_THRESHOLD) && currentAnimationPos.y>(
                endPos.y - ANIMATION_THRESHOLD) && currentAnimationPos.y < (endPos.y + ANIMATION_THRESHOLD)) {

        animationPoints.erase(animationPoints.begin()); // Remove first point, since reached
        if (!animationPoints.empty()) {
            currentAnimationDir = animationPoints.front() - currentAnimationPos; // Update animation direction
        }
    }

    auto circliename = circleTextures.find(game.getCurrentLevel().getGraph().getNode("QUEUE_0")->getColor().colorName);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,circliename->second);
    glUniform1i(texUniform,1);
    drawCircle(currentAnimationPos.x, currentAnimationPos.y);
}


void Graphics::drawLine() {
    GLuint lineBuffer;
    glGenBuffers(1, &lineBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, lineBuffer);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(lines[0]), &lines[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_LINES, 0, lines.size() / 3);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &lineBuffer);
}

void Graphics::loadTexture(string filePath, GLuint& texture) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filePath.c_str(),
                                    &width, &height, &nrChannels, 0);

    if (data == nullptr) {
        throw std::logic_error("Texture file coudn't be read.");
    } else {
        GLint internalformat;
        GLenum format;
        switch (nrChannels) {
            case 1:
                internalformat = GL_R8;
                format = GL_RED;
                break;
            case 2:
                internalformat = GL_RG8;
                format = GL_RG;
                break;
            case 3:
                internalformat = GL_RGB8;
                format = GL_RGB;
                break;
            case 4:
                internalformat = GL_RGBA8;
                format = GL_RGBA;
                break;
            default:
                internalformat = GL_RGB8;
                format = GL_RGB;
                break;
        }

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_REPEAT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
    }
}

void Graphics::draw() {
    map<string, Node> nodes = game.getCurrentLevel().getGraph().getNodes();

    double t_frame = 0;
    if (animationInProgress) {
        std::chrono::time_point<std::chrono::system_clock> t_current = std::chrono::system_clock::now();
        std::chrono::duration<double> t_elapsed = t_current - t_start;
        t_frame = t_elapsed.count();
        t_start = t_current;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    check();

    // could also be set just once
    // unless different ones are used
    glUseProgram(program);
    check();

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(aProjection, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(aView, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(aModel, 1, GL_FALSE, &model[0][0]);
    check();


    GLfloat backgroundData[] = {
            screenToWorld(0, 0).first, screenToWorld(0, 0).second, 0.0, 0.0,
            screenToWorld(_width, 0).first, screenToWorld(_width, 0).second, 1.0, 0.0,
            screenToWorld(_width, _height).first, screenToWorld(_width, _height).second, 1.0, 1.0,
            screenToWorld(0, _height).first, screenToWorld(0, _height).second, 0.0, 1.0,
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,textureScene);
    glUniform1i(texUniform,0);
    drawSquare(backgroundData);

    // a square as a simple triangle fan
    // (it shows as a rectangle without a projection matrix ;) )
    GLfloat squareData[] = {
            -4 * spacingX, -6.6f * spacingY + offsetY, 0.0, 1.0, //0.0, 0.0,// 1.0f, 1.0f,
            4 * spacingX, -6.6f * spacingY + offsetY, 1.0, 1.0,//1.0, 0.0,// 1.0f, 0.0f,
            4 * spacingX, 6.6f * spacingY + offsetY, 1.0, 0.0,//1.0, 1.0,// 0.0f, 0.0f,
            -4 * spacingX, 6.6f * spacingY + offsetY, 0.0, 0.0//0.0, 1.0 //0.0f, 1.0f
    };



    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,texturePlayfield);
    glUniform1i(texUniform,1);
    drawSquare(squareData);


    glUniform4f(uColor, 0.3176, 0.6118, 0.8588, 1.0);
    drawSquare(squareData);

    glUniform4f(uColor, lineColor.x, lineColor.y, lineColor.z, 1.0);
    drawLine();


    drawLine();
    for (const pair<string, Node> &node: nodes) {
        if (node.first != "ROOT") {
            if (node.first == "QUEUE_0") {
                if (animationInProgress) {
                    drawAnimatedShootCircle(node.second.getColor(), t_frame);
                } else {
                    drawCircleByName(node.first, node.second.getColor());
                }
                continue;
            }
            drawCircleByName(node.first, node.second.getColor());
        }
    }

    check();

    // finalize
    glFlush();
    glFinish();
    check();

    // swap buffers: show what we just painted
    eglSwapBuffers(display, surface);
    check();
}

void Graphics::startAnimation() {
    animationInProgress = true;

    // Add final pos to end of animation points
    auto endPos = nodePositions.find(nodeToAdd)->second;
    animationPoints.emplace_back(endPos.first, endPos.second, 0.0f);

    // Prepare starting point of animation
    auto startPos = nodePositions.find("QUEUE_0")->second;
    currentAnimationPos = glm::vec3(startPos.first, startPos.second, 0.0f);

    // Prepare animation initial direction
    currentAnimationDir = animationPoints.front() - currentAnimationPos;

    t_start = std::chrono::system_clock::now();
}

void Graphics::stopAnimation() {
    animationInProgress = false;

    game.shoot(nodeToAdd[0], (int) nodeToAdd[1] - 48);
    if (game.getCurrentLevel().isWon()) {
        cout << "WINNER WINNER CHICKEN DINNER!" << endl;
        game.nextLevel();
        circleTextures.clear();
        initCircleTextures();
    }
    if (game.getCurrentLevel().isGameOver()) {
        cout << "GAME OVER!" << endl;
        exit(0);
    }
}
