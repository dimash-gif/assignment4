#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace glm;

// --- Function Prototypes ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
string loadShaderFromFile(const string& filePath);
GLuint compileShader(GLenum type, const char* src);
GLuint makeProgram(const string& vertexPath, const string& fragmentPath);
void updatePatchGeometry();
vec3 evaluateBezierCurve(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t);
vec3 evaluateBezierPatch(float u, float v);

// --- Window ---
int windowWidth = 800, windowHeight = 600;

// --- Shaders ---
GLuint patchShader, simpleShader;

// --- Geometry ---
vector<vec3> patchVertices;
GLuint patchVAO = 0, patchVBO = 0;
GLuint controlPointsVAO = 0, controlPointsVBO = 0;
GLuint axesVAO = 0, axesVBO = 0;

// --- Camera ---
float camAngle = 45.0f, camPitch = 30.0f, camDist = 8.0f;
vec3 patchCenter = vec3(0.0f, 0.0f, 0.0f);

// --- Control Points & Tessellation ---
vector<vec3> controlPoints = {
    vec3(-1.5, -1.5,  1.5), vec3(-0.5, -1.5,  1.5), vec3(0.5, -1.5,  1.5), vec3(1.5, -1.5,  1.5),
    vec3(-1.5, -0.5,  1.5), vec3(-0.5, -0.5, -1.0), vec3(0.5, -0.5, -1.0), vec3(1.5, -0.5,  1.5),
    vec3(-1.5,  0.5,  1.5), vec3(-0.5,  0.5, -1.0), vec3(0.5,  0.5, -1.0), vec3(1.5,  0.5,  1.5),
    vec3(-1.5,  1.5,  1.5), vec3(-0.5,  1.5,  1.5), vec3(0.5,  1.5,  1.5), vec3(1.5,  1.5,  1.5)
};
int tessellationLevel = 10;
int selectedControlPoint = 0;

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Assignment 4 - Part 1: Bezier Patch", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { cout << "Failed to initialize GLAD" << endl; return -1; }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    patchShader = makeProgram("shaders/phong.vert", "shaders/phong.frag");
    simpleShader = makeProgram("shaders/simple.vert", "shaders/simple.frag");

    glGenVertexArrays(1, &patchVAO);
    glGenBuffers(1, &patchVBO);
    glGenVertexArrays(1, &controlPointsVAO);
    glGenBuffers(1, &controlPointsVBO);
    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);

    updatePatchGeometry();

    float axesData[] = {
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };
    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesData), axesData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- MODIFIED: Added instructions for number keys ---
    cout << "--- Assignment 4, Part 1: Bezier Patch ---\n"
         << "Controls:\n"
         << "  ESC: Close Window\n"
         << "  W/S: Camera Pitch\n"
         << "  A/D: Camera Angle\n"
         << "  Z/X: Camera Zoom\n"
         << "  R: Reset View\n"
         << "  0-9: Select Control Points 0-9\n"
         << "  LEFT/RIGHT: Cycle Through Control Points\n"
         << "  U/J (X), I/K (Y), O/L (Z): Move Control Point\n"
         << "  +/-: Change Tessellation Level\n";

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float camX = camDist * cos(radians(camAngle)) * cos(radians(camPitch));
        float camY = camDist * sin(radians(camPitch));
        float camZ = camDist * sin(radians(camAngle)) * cos(radians(camPitch));
        vec3 camPos = vec3(camX, camY, camZ);
        mat4 view = lookAt(camPos, patchCenter, vec3(0.0, 1.0, 0.0));
        mat4 projection = perspective(radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
        
        glEnable(GL_DEPTH_TEST);
        glUseProgram(patchShader);
        glUniformMatrix4fv(glGetUniformLocation(patchShader, "view"), 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(patchShader, "projection"), 1, GL_FALSE, value_ptr(projection));
        glUniform3fv(glGetUniformLocation(patchShader, "lightPos"), 1, value_ptr(camPos));
        glUniform3f(glGetUniformLocation(patchShader, "lightColor"), 1.0f, 1.0f, 1.0f);
        glBindVertexArray(patchVAO);
        glDrawArrays(GL_TRIANGLES, 0, patchVertices.size() / 2);

        glDisable(GL_DEPTH_TEST);
        glUseProgram(simpleShader);
        glUniformMatrix4fv(glGetUniformLocation(simpleShader, "view"), 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(simpleShader, "projection"), 1, GL_FALSE, value_ptr(projection));
        
        glBindVertexArray(controlPointsVAO);
        for(int i = 0; i < 16; ++i) {
            if (i == selectedControlPoint) {
                glPointSize(25.0f);
                glUniform3f(glGetUniformLocation(simpleShader, "uColor"), 1.0f, 1.0f, 0.0f);
            } else {
                glPointSize(15.0f);
                glUniform3f(glGetUniformLocation(simpleShader, "uColor"), 1.0f, 0.5f, 0.0f);
            }
            glDrawArrays(GL_POINTS, i, 1);
        }

        glLineWidth(3.0f);
        glBindVertexArray(axesVAO);
        glDrawArrays(GL_LINES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// --- MODIFIED: Added key handling for numbers 0-9 ---
void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    bool needsUpdate = false;
    float step = 0.1f;
    
    if (key == GLFW_KEY_W) camPitch = glm::min(89.0f, camPitch + 2.0f);
    if (key == GLFW_KEY_S) camPitch = glm::max(-89.0f, camPitch - 2.0f);
    if (key == GLFW_KEY_A) camAngle -= 2.0f;
    if (key == GLFW_KEY_D) camAngle += 2.0f;
    if (key == GLFW_KEY_Z) camDist = glm::max(0.5f, camDist - 0.2f);
    if (key == GLFW_KEY_X) camDist += 0.2f;
    if (key == GLFW_KEY_R) { camAngle = 45.0f; camPitch = 30.0f; camDist = 8.0f; }

    if (key == GLFW_KEY_LEFT) selectedControlPoint = (selectedControlPoint - 1 + 16) % 16;
    if (key == GLFW_KEY_RIGHT) selectedControlPoint = (selectedControlPoint + 1) % 16;

    // --- NEW: Handle number keys to select a control point directly ---
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
        selectedControlPoint = key - GLFW_KEY_0;
    }
    // --- END NEW ---

    if (key == GLFW_KEY_U) { controlPoints[selectedControlPoint].x += step; needsUpdate = true; }
    if (key == GLFW_KEY_J) { controlPoints[selectedControlPoint].x -= step; needsUpdate = true; }
    if (key == GLFW_KEY_I) { controlPoints[selectedControlPoint].y += step; needsUpdate = true; }
    if (key == GLFW_KEY_K) { controlPoints[selectedControlPoint].y -= step; needsUpdate = true; }
    if (key == GLFW_KEY_O) { controlPoints[selectedControlPoint].z += step; needsUpdate = true; }
    if (key == GLFW_KEY_L) { controlPoints[selectedControlPoint].z -= step; needsUpdate = true; }

    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) { tessellationLevel = glm::min(100, tessellationLevel + 1); needsUpdate = true; }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) { if (tessellationLevel > 1) { tessellationLevel--; needsUpdate = true; } }

    if(needsUpdate) updatePatchGeometry();
}

// ... (The rest of the file is unchanged) ...
void updatePatchGeometry() {
    patchVertices.clear();
    float step = 1.0f / tessellationLevel;

    for (int i = 0; i < tessellationLevel; ++i) {
        for (int j = 0; j < tessellationLevel; ++j) {
            float u0 = i * step, v0 = j * step;
            float u1 = (i + 1) * step, v1 = (j + 1) * step;

            vec3 p00 = evaluateBezierPatch(u0, v0);
            vec3 p10 = evaluateBezierPatch(u1, v0);
            vec3 p01 = evaluateBezierPatch(u0, v1);
            vec3 p11 = evaluateBezierPatch(u1, v1);

            vec3 n1 = normalize(cross(p10 - p00, p01 - p00));
            patchVertices.push_back(p00); patchVertices.push_back(n1);
            patchVertices.push_back(p10); patchVertices.push_back(n1);
            patchVertices.push_back(p01); patchVertices.push_back(n1);
            
            vec3 n2 = normalize(cross(p10 - p11, p01 - p11));
            patchVertices.push_back(p10); patchVertices.push_back(n2);
            patchVertices.push_back(p11); patchVertices.push_back(n2);
            patchVertices.push_back(p01); patchVertices.push_back(n2);
        }
    }

    glBindVertexArray(patchVAO);
    glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
    glBufferData(GL_ARRAY_BUFFER, patchVertices.size() * sizeof(vec3), patchVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(controlPointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, controlPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(vec3), controlPoints.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

vec3 evaluateBezierCurve(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t) {
    vec3 p01 = mix(p0, p1, t);
    vec3 p12 = mix(p1, p2, t);
    vec3 p23 = mix(p2, p3, t);
    vec3 p012 = mix(p01, p12, t);
    vec3 p123 = mix(p12, p23, t);
    return mix(p012, p123, t);
}

vec3 evaluateBezierPatch(float u, float v) {
    vec3 v_curve_points[4];
    for (int i = 0; i < 4; ++i) {
        v_curve_points[i] = evaluateBezierCurve(
            controlPoints[i * 4 + 0], controlPoints[i * 4 + 1],
            controlPoints[i * 4 + 2], controlPoints[i * 4 + 3], u);
    }
    return evaluateBezierCurve(v_curve_points[0], v_curve_points[1], v_curve_points[2], v_curve_points[3], v);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = width; windowHeight = height;
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
