#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

using namespace std;
using namespace glm;

// --- Structs ---
struct SceneObject {
    GLuint VAO = 0;
    int vertexCount = 0;
    mat4 modelMatrix = mat4(1.0f);
    vec3 diffuseColor = vec3(1.0f);
    vec3 pickingColor = vec3(0.0f);
};

// --- Function Prototypes ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
string loadShaderFromFile(const string& filePath);
GLuint compileShader(GLenum type, const char* src);
GLuint makeProgram(const string& vertexPath, const string& fragmentPath);
void setupFBO();
void generateSphere(vector<float>& vertices, float radius, int sectorCount, int stackCount);
void generateSmoothCube(vector<float>& vertices, float size);
void generateCone(vector<float>& vertices, float radius, float height, int sectorCount);
void performPicking(double mouseX, double mouseY);

// --- Global State ---
int windowWidth = 800, windowHeight = 600;
bool antiAliasing = false;

// --- Shaders ---
GLuint smoothPhongShader, pickingShader;

// --- Camera ---
float camAngle = 20.0f, camPitch = 20.0f, camDist = 10.0f;
vec3 lookAtPoint = vec3(0.0f, 0.0f, 0.0f);

// --- Scene Data ---
GLuint pickingFBO = 0, pickingTexture = 0;
GLuint depthRenderbuffer = 0;
map<int, SceneObject> sceneObjects;

int main() {
    srand(time(NULL));
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Assignment 4 - Part 2: Picking", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { cout << "Failed to initialize GLAD" << endl; return -1; }

    glEnable(GL_DEPTH_TEST);

    smoothPhongShader = makeProgram("shaders/smooth_phong.vert", "shaders/smooth_phong.frag");
    pickingShader = makeProgram("shaders/picking.vert", "shaders/picking.frag");

    sceneObjects[1] = SceneObject();
    sceneObjects[2] = SceneObject();
    sceneObjects[3] = SceneObject();

    sceneObjects[1].modelMatrix = translate(mat4(1.0f), vec3(-2.5, 0, 0));
    sceneObjects[1].diffuseColor = vec3(0.8, 0.2, 0.2);
    sceneObjects[1].pickingColor = vec3(1.0, 0.0, 0.0);

    sceneObjects[2].modelMatrix = translate(mat4(1.0f), vec3(0, 0, 0));
    sceneObjects[2].diffuseColor = vec3(0.2, 0.8, 0.2);
    sceneObjects[2].pickingColor = vec3(0.0, 1.0, 0.0);

    sceneObjects[3].modelMatrix = translate(mat4(1.0f), vec3(2.5, -0.5, 0));
    sceneObjects[3].diffuseColor = vec3(0.2, 0.2, 0.8);
    sceneObjects[3].pickingColor = vec3(0.0, 0.0, 1.0);

    vector<float> sphereVertices, cubeVertices, coneVertices;
    generateSphere(sphereVertices, 0.8f, 36, 18);
    generateSmoothCube(cubeVertices, 1.2f);
    generateCone(coneVertices, 0.7f, 1.5f, 36);
    
    for (auto const& [id, obj] : sceneObjects) {
        glGenVertexArrays(1, &sceneObjects[id].VAO);
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindVertexArray(sceneObjects[id].VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        if (id == 1) {
            glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
            sceneObjects[id].vertexCount = sphereVertices.size() / 6;
        } else if (id == 2) {
            glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
            sceneObjects[id].vertexCount = cubeVertices.size() / 6;
        } else {
            glBufferData(GL_ARRAY_BUFFER, coneVertices.size() * sizeof(float), coneVertices.data(), GL_STATIC_DRAW);
            sceneObjects[id].vertexCount = coneVertices.size() / 6;
        }
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    
    setupFBO();

    cout << "--- Assignment 4, Part 2: Picking ---\n"
         << "Controls:\n"
         << "  ESC: Close Window\n"
         << "  Click: Pick an object to change its color\n"
         << "  A: Toggle Anti-aliasing\n"
         << "  W/S/D/Q: Camera Control\n"
         << "  Z/X: Camera Zoom\n"
         << "  R: Reset View\n";

    while (!glfwWindowShouldClose(window)) {
        if (antiAliasing) glEnable(GL_MULTISAMPLE); else glDisable(GL_MULTISAMPLE);
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float camX = camDist * cos(radians(camAngle)) * cos(radians(camPitch));
        float camY = camDist * sin(radians(camPitch));
        float camZ = camDist * sin(radians(camAngle)) * cos(radians(camPitch));
        vec3 camPos = lookAtPoint + vec3(camX, camY, camZ);
        mat4 view = lookAt(camPos, lookAtPoint, vec3(0.0, 1.0, 0.0));
        mat4 projection = perspective(radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

        glUseProgram(smoothPhongShader);
        glUniformMatrix4fv(glGetUniformLocation(smoothPhongShader, "view"), 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(smoothPhongShader, "projection"), 1, GL_FALSE, value_ptr(projection));
        glUniform3fv(glGetUniformLocation(smoothPhongShader, "lightPos"), 1, value_ptr(camPos));
        glUniform3f(glGetUniformLocation(smoothPhongShader, "lightColor"), 1.0f, 1.0f, 1.0f);

        for (auto const& [id, obj] : sceneObjects) {
            glUniformMatrix4fv(glGetUniformLocation(smoothPhongShader, "model"), 1, GL_FALSE, value_ptr(obj.modelMatrix));
            glUniform3fv(glGetUniformLocation(smoothPhongShader, "objectColor"), 1, value_ptr(obj.diffuseColor));
            glBindVertexArray(obj.VAO);
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int, int action, int) {
    // --- ADDED ---
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (key == GLFW_KEY_A) { antiAliasing = !antiAliasing; cout << "Anti-aliasing: " << (antiAliasing ? "ON" : "OFF") << endl; }
    
    if (key == GLFW_KEY_W) camPitch = glm::min(89.0f, camPitch + 2.0f);
    if (key == GLFW_KEY_S) camPitch = glm::max(-89.0f, camPitch - 2.0f);
    if (key == GLFW_KEY_D) camAngle -= 2.0f;
    if (key == GLFW_KEY_Q) camAngle += 2.0f;
    if (key == GLFW_KEY_Z) camDist = glm::max(0.5f, camDist - 0.2f);
    if (key == GLFW_KEY_X) camDist += 0.2f;
    if (key == GLFW_KEY_R) { camAngle = 20.0f; camPitch = 20.0f; camDist = 10.0f; }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        performPicking(xpos, ypos);
    }
}

void performPicking(double mouseX, double mouseY) {
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
    glViewport(0, 0, windowWidth, windowHeight);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_MULTISAMPLE);

    glUseProgram(pickingShader);
    float camX = camDist * cos(radians(camAngle)) * cos(radians(camPitch));
    float camY = camDist * sin(radians(camPitch));
    float camZ = camDist * sin(radians(camAngle)) * cos(radians(camPitch));
    vec3 camPos = lookAtPoint + vec3(camX, camY, camZ);
    mat4 view = lookAt(camPos, lookAtPoint, vec3(0.0, 1.0, 0.0));
    mat4 projection = perspective(radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(pickingShader, "view"), 1, GL_FALSE, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(pickingShader, "projection"), 1, GL_FALSE, value_ptr(projection));

    for (auto const& [id, obj] : sceneObjects) {
        glUniformMatrix4fv(glGetUniformLocation(pickingShader, "model"), 1, GL_FALSE, value_ptr(obj.modelMatrix));
        glUniform3fv(glGetUniformLocation(pickingShader, "pickingColor"), 1, value_ptr(obj.pickingColor));
        glBindVertexArray(obj.VAO);
        glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount);
    }

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    unsigned char pixel[4];
    glReadPixels(mouseX, windowHeight - mouseY -1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);
    if (antiAliasing) glEnable(GL_MULTISAMPLE);

    int pickedID = 0;
    if (pixel[0] == 255) pickedID = 1;
    else if (pixel[1] == 255) pickedID = 2;
    else if (pixel[2] == 255) pickedID = 3;

    if (pickedID != 0) {
        cout << "Picked object with ID: " << pickedID << endl;
        sceneObjects[pickedID].diffuseColor = vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
    }
}

void setupFBO() {
    if (pickingFBO) glDeleteFramebuffers(1, &pickingFBO);
    if (pickingTexture) glDeleteTextures(1, &pickingTexture);
    if (depthRenderbuffer) glDeleteRenderbuffers(1, &depthRenderbuffer);

    glGenFramebuffers(1, &pickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

    glGenTextures(1, &pickingTexture);
    glBindTexture(GL_TEXTURE_2D, pickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);

    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void generateSphere(vector<float>& vertices, float radius, int sectorCount, int stackCount) {
    vertices.clear();
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    
    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    vector<float> tempVertices;
    for(int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);
        for(int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            tempVertices.push_back(x);
            tempVertices.push_back(y);
            tempVertices.push_back(z);
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            tempVertices.push_back(nx);
            tempVertices.push_back(ny);
            tempVertices.push_back(nz);
        }
    }

    int k1, k2;
    for(int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;
        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                vertices.insert(vertices.end(), {tempVertices[k1*6], tempVertices[k1*6+1], tempVertices[k1*6+2], tempVertices[k1*6+3], tempVertices[k1*6+4], tempVertices[k1*6+5]});
                vertices.insert(vertices.end(), {tempVertices[k2*6], tempVertices[k2*6+1], tempVertices[k2*6+2], tempVertices[k2*6+3], tempVertices[k2*6+4], tempVertices[k2*6+5]});
                vertices.insert(vertices.end(), {tempVertices[(k1+1)*6], tempVertices[(k1+1)*6+1], tempVertices[(k1+1)*6+2], tempVertices[(k1+1)*6+3], tempVertices[(k1+1)*6+4], tempVertices[(k1+1)*6+5]});
            }
            if (i != (stackCount-1)) {
                 vertices.insert(vertices.end(), {tempVertices[(k1+1)*6], tempVertices[(k1+1)*6+1], tempVertices[(k1+1)*6+2], tempVertices[(k1+1)*6+3], tempVertices[(k1+1)*6+4], tempVertices[(k1+1)*6+5]});
                 vertices.insert(vertices.end(), {tempVertices[k2*6], tempVertices[k2*6+1], tempVertices[k2*6+2], tempVertices[k2*6+3], tempVertices[k2*6+4], tempVertices[k2*6+5]});
                 vertices.insert(vertices.end(), {tempVertices[(k2+1)*6], tempVertices[(k2+1)*6+1], tempVertices[(k2+1)*6+2], tempVertices[(k2+1)*6+3], tempVertices[(k2+1)*6+4], tempVertices[(k2+1)*6+5]});
            }
        }
    }
}

void generateSmoothCube(vector<float>& vertices, float size) {
    float s = size / 2.0f;
    vertices = {
        -s, -s, -s, -0.577f, -0.577f, -0.577f,  s, -s, -s,  0.577f, -0.577f, -0.577f,  s,  s, -s,  0.577f,  0.577f, -0.577f,
        -s, -s, -s, -0.577f, -0.577f, -0.577f,  s,  s, -s,  0.577f,  0.577f, -0.577f, -s,  s, -s, -0.577f,  0.577f, -0.577f,
        -s, -s,  s, -0.577f, -0.577f,  0.577f,  s, -s,  s,  0.577f, -0.577f,  0.577f,  s,  s,  s,  0.577f,  0.577f,  0.577f,
        -s, -s,  s, -0.577f, -0.577f,  0.577f,  s,  s,  s,  0.577f,  0.577f,  0.577f, -s,  s,  s, -0.577f,  0.577f,  0.577f,
        -s,  s,  s, -0.577f,  0.577f,  0.577f, -s,  s, -s, -0.577f,  0.577f, -0.577f, -s, -s, -s, -0.577f, -0.577f, -0.577f,
        -s,  s,  s, -0.577f,  0.577f,  0.577f, -s, -s, -s, -0.577f, -0.577f, -0.577f, -s, -s,  s, -0.577f, -0.577f,  0.577f,
         s,  s,  s,  0.577f,  0.577f,  0.577f,  s,  s, -s,  0.577f,  0.577f, -0.577f,  s, -s, -s,  0.577f, -0.577f, -0.577f,
         s,  s,  s,  0.577f,  0.577f,  0.577f,  s, -s, -s,  0.577f, -0.577f, -0.577f,  s, -s,  s,  0.577f, -0.577f,  0.577f,
        -s, -s, -s, -0.577f, -0.577f, -0.577f,  s, -s, -s,  0.577f, -0.577f, -0.577f,  s, -s,  s,  0.577f, -0.577f,  0.577f,
        -s, -s, -s, -0.577f, -0.577f, -0.577f,  s, -s,  s,  0.577f, -0.577f,  0.577f, -s, -s,  s, -0.577f, -0.577f,  0.577f,
        -s,  s, -s, -0.577f,  0.577f, -0.577f,  s,  s, -s,  0.577f,  0.577f, -0.577f,  s,  s,  s,  0.577f,  0.577f,  0.577f,
        -s,  s, -s, -0.577f,  0.577f, -0.577f,  s,  s,  s,  0.577f,  0.577f,  0.577f, -s,  s,  s, -0.577f,  0.577f,  0.577f
    };
}

void generateCone(vector<float>& vertices, float radius, float height, int sectorCount) {
    vertices.clear();
    float sectorStep = 2 * M_PI / sectorCount;
    vec3 tip(0, height, 0);

    for (int i = 0; i < sectorCount; ++i) {
        float angle1 = i * sectorStep;
        float angle2 = (i + 1) * sectorStep;
        
        vec3 p1(radius * cos(angle1), 0.0f, radius * sin(angle1));
        vec3 p2(radius * cos(angle2), 0.0f, radius * sin(angle2));

        vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f});
        vertices.insert(vertices.end(), {p1.x, p1.y, p1.z, 0.0f, -1.0f, 0.0f});
        vertices.insert(vertices.end(), {p2.x, p2.y, p2.z, 0.0f, -1.0f, 0.0f});

        vec3 normal = normalize(cross(p2 - p1, tip - p1));
        vertices.insert(vertices.end(), {p1.x, p1.y, p1.z, normal.x, normal.y, normal.z});
        vertices.insert(vertices.end(), {p2.x, p2.y, p2.z, normal.x, normal.y, normal.z});
        vertices.insert(vertices.end(), {tip.x, tip.y, tip.z, normal.x, normal.y, normal.z});
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = width; windowHeight = height;
    setupFBO();
}

string loadShaderFromFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) { cerr << "Failed to open shader file: " << filePath << endl; return ""; }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[512]; glGetShaderInfoLog(s, 512, NULL, log); cerr << "Shader Compile Error: " << log << endl; }
    return s;
}

GLuint makeProgram(const string& vertexPath, const string& fragmentPath) {
    string vCode = loadShaderFromFile(vertexPath);
    string fCode = loadShaderFromFile(fragmentPath);
    GLuint v = compileShader(GL_VERTEX_SHADER, vCode.c_str());
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fCode.c_str());
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { char log[512]; glGetProgramInfoLog(prog, 512, NULL, log); cerr << "Shader Link Error: " << log << endl; }
    glDeleteShader(v); glDeleteShader(f);
    return prog;
}
