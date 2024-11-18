#include "engine.h"

enum state {start, play, over};
state screen;
const int ON = 1;
const int OFF = 0;

// Colors
color originalFill, hoverFill, pressFill, lightOn, lightOff;

const color yellow = {1,1,0};
const color gray = {0.52,0.52, 0.52};
const color red = {1,0,0};

// TODO Note: complete the drawing TODOs in render before the other TODOs,
//  otherwise you won't be able to see if your code is correct

Engine::Engine() : keys(), lightsOn() {
    this->initWindow();
    this->initShaders();
    this->initShapes();

    originalFill = {1, 0, 0, 1};
    hoverFill.vec = originalFill.vec + vec4{0.5, 0.5, 0.5, 0};
    pressFill.vec = originalFill.vec - vec4{0.5, 0.5, 0.5, 0};
    lightOn = yellow;
    lightOff = gray;
/*
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++) {
            vec2 lightPos(width * (x + 1) / 6, height * (y + 1) / 6);
            lights[x][y] = make_unique<Rect>(Rect(shapeShader, lightPos, vec2(80,80), yellow));
        }
    }
    */
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.use().setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    // red spawn button centered in the top left corner
    //spawnButton = make_unique<Rect>(shapeShader, vec2{width/2,height/2}, vec2{100, 50}, color{1, 0, 0, 1});

    int xPos = 100;

    for (int i = 0; i < 25; i+= 5) {
        lights.push_back(make_unique<Rect>(shapeShader, vec2{xPos, height - height*(.1)}, vec2{100, 100},
                                           yellow, i, ON));
        lights.push_back(make_unique<Rect>(shapeShader, vec2{xPos, height - height*(.3)}, vec2{100, 100},
                                         yellow, i+1, ON));
        lights.push_back(make_unique<Rect>(shapeShader, vec2{xPos, height - height*(.5)}, vec2{100, 100},
                                         yellow, i+2, ON));
        lights.push_back(make_unique<Rect>(shapeShader, vec2{xPos, height - height*(.7)}, vec2{100, 100},
                                         yellow, i+3, ON));
        lights.push_back(make_unique<Rect>(shapeShader, vec2{xPos, height - height*(.9)}, vec2{100, 100},
                                           yellow, i+4, ON));
        xPos += 130;
    }

    for (int i = 0; i < lights.size(); i++) {
        lightBackgrounds.push_back(make_unique<Rect>(shapeShader, lights[i]->getPos(), vec2(107, 107), BLACK, i, OFF));
    }

}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // TODO: If we're in the start screen and the user presses s, change screen to play
    // Hint: The index is GLFW_KEY_S
    if (screen == start and keys[GLFW_KEY_S]) {
        screen = play;
        startTime = glfwGetTime();
    }

    // Mouse position is inverted because the origin of the window is in the top left corner
    MouseY = height - MouseY; // Invert y-axis of mouse position

    // Set color of light to their state, either 0 or 1
    if (screen == play) {
        for (int i = 0; i < 25; i++){
            if (lights[i]->getState() == 1) {
                lights[i]->setColor(lightOn);
            }
            if (lights[i]->getState() == 0) {
                lights[i]->setColor(lightOff);
            }

        }
    }

    bool buttonOverlapsMouse;
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // TODO: When in play screen, if the user hovers or clicks on the button then change the Boxes's color
    if (screen == play) {
        for (int i = 0; i < 25; i++){
            buttonOverlapsMouse = lights[i]->isOverlapping(vec2(MouseX, MouseY));
            if (!mousePressed && mousePressedLastFrame && buttonOverlapsMouse) {
                lights[i]->flipState();
                if (i < 20) {
                    lights[i+5]->flipState();
                }
                if (i > 4) {
                    lights [i-5]->flipState();
                }
                if (i % 5 > 0) {
                    lights [i -1]->flipState();
                }
                if (i != 4 && i != 9 && i != 14 && i !=19 && i != 24) {
                    lights[i+1]->flipState();
                }
            }
            if (buttonOverlapsMouse) {
                // DONE: Red Borders
                lightBackgrounds[i]->setColor(red);
            }
            else {
                lightBackgrounds[i]->setColor(BLACK);
            }
        }
    }
    // Save mousePressed for next frame
    mousePressedLastFrame = mousePressed;

}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // TODO: End the game all light are out
    bool endGame = true;
    for(int i = 0; i < 25; i++) {
        if (lights[i]->getState() == 1){
            endGame = false;
        }
    }
    if (endGame) {
        for (const auto& light : lights) {
            light->setState(OFF);
            light->setColor(lightOff);
        }
        if (screen != over)
            endTime = glfwGetTime();
        screen = over;
    }

    // If the size of the confetti vector reaches 100, change screen to over

}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to draw shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            string message = "Press s to start";
            // (12 * message.length()) is the offset to center text.
            // 12 pixels is the width of each character scaled by 1.
            // NOTE: This line changes the shader being used to the font shader.
            //  If you want to draw shapes again after drawing text,
            //  you'll need to call shapeShader.use() again first.

            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, projection, 1, vec3{1, 1, 1});

            break;
        }
        case play: {

            for (int i = 0; i < lights.size(); i++) {
                lightBackgrounds[i]->setUniforms();
                lightBackgrounds[i]->draw();
                lights[i]->setUniforms();
                lights[i]->draw();

            }
            /*
            for (auto & light : lights) {
                for (const auto & y : light) {
                    y->setUniforms();
                    y->draw();
                }
            }
             */

            break;
        }
        case over: {
            for (int i = 0; i < lights.size(); i++) {
                lights[i]->setUniforms();
                lights[i]->draw();
            }

            // Source: https://en.cppreference.com/w/cpp/string/basic_string/to_string
            string message = "You win! Time=" + std::to_string(int(endTime - startTime)) + "s";
            // TODO: Display the message on the screen
            fontRenderer->renderText(message,width/2 - (12 * message.length()), height/2, projection, 1, vec3{1,1,1});

            break;
        }
    }

    glfwSwapBuffers(window);
}


bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}