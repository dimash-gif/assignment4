// --- START OF FINAL MODIFIED FILE my-project-master/src/main_part1.cpp ---

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
#include <cmath>

using namespace std;
using namespace glm;

// --- Function Prototypes ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
string loadShaderFromFile(const string& filePath);
GLuint compileShader(GLenum type, const char* src);
GLuint makeProgram(const string& vertexPath, const string& fragmentPath);
void updatePatchGeometry();
vec3 evaluateBezierCurve(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t);
vec3 evaluateBezierPatch(float u, float v);

// --- Globals ---
int windowWidth = 800, windowHeight = 600;
GLuint patchShader;
vector<float> patchVertices;
GLuint patchVAO = 0, patchVBO = 0;
float camAngle = 45.0f, camPitch = 30.0f, camDist = 8.0f;
int tessellationLevel = 150;

vector<vec3> controlPoints = {
    vec3(-1.5, -1.5, -1.0), vec3(-0.5, -1.5, -1.0), vec3(0.5, -1.5, -1.0), vec3(1.5, -1.5, -1.0),
    vec3(-1.5, -0.5, -1.0), vec3(-0.5,  0.5,  2.0), vec3(0.5,  0.5,  2.0), vec3(1.5, -0.5, -1.0),
    vec3(-1.5,  0.5, -1.0), vec3(-0.5,  0.5,  2.0), vec3(0.5,  0.5,  2.0), vec3(1.5,  0.5, -1.0),
    vec3(-1.5,  1.5, -1.0), vec3(-0.5,  1.5, -1.0), vec3(0.5,  1.5, -1.0), vec3(1.5,  1.5, -1.0)
};

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Bezier Patch with Procedural Texture", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { cout << "Failed to initialize GLAD" << endl; return -1; }

    glEnable(GL_DEPTH_TEST);

    patchShader = makeProgram("shaders/procedural_patch.vert", "shaders/procedural_patch.frag");
    
    glGenVertexArrays(1, &patchVAO);
    glGenBuffers(1, &patchVBO);
    updatePatchGeometry();

    cout << "--- Bezier Patch with Procedural Rings Texture ---\n" << "Controls: W/S/A/D to orbit camera, Z/X to zoom.\n";

    while (!glfwWindowShouldClose(window)) {
        // --- Input (Camera Control) ---
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPitch = glm::min(89.0f, camPitch + 2.0f);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPitch = glm::max(-89.0f, camPitch - 2.0f);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camAngle -= 2.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camAngle += 2.0f;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) camDist = glm::max(0.5f, camDist - 0.2f);
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) camDist += 0.2f;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        // --- Rendering ---
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float camX = camDist * cos(radians(camAngle)) * cos(radians(camPitch));
        float camY = camDist * sin(radians(camPitch));
        float camZ = camDist * sin(radians(camAngle)) * cos(radians(camPitch));
        vec3 camPos = vec3(camX, camY, camZ);
        mat4 view = lookAt(camPos, vec3(0.0), vec3(0.0, 1.0, 0.0));
        mat4 projection = perspective(radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
        
        glUseProgram(patchShader);
        glUniformMatrix4fv(glGetUniformLocation(patchShader, "model"), 1, GL_FALSE, value_ptr(mat4(1.0f)));
        glUniformMatrix4fv(glGetUniformLocation(patchShader, "view"), 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(patchShader, "projection"), 1, GL_FALSE, value_ptr(projection));
        glUniform3fv(glGetUniformLocation(patchShader, "viewPos"), 1, value_ptr(camPos));
        glUniform3f(glGetUniformLocation(patchShader, "lightPos"), 0.0f, 2.0f, 5.0f);
        glUniform1f(glGetUniformLocation(patchShader, "shininess"), 256.0f);

        glBindVertexArray(patchVAO);
        glDrawArrays(GL_TRIANGLES, 0, tessellationLevel * tessellationLevel * 6);

        // --- REMOVED: All code for drawing control points and axes is gone ---

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void updatePatchGeometry() {
    patchVertices.clear();
    float step = 1.0f / (float)tessellationLevel;
    for (int i = 0; i < tessellationLevel; ++i) {
        for (int j = 0; j < tessellationLevel; ++j) {
            float u0 = (float)i * step, v0 = (float)j * step;
            float u1 = (float)(i + 1) * step, v1 = (float)(j + 1) * step;
            vec3 p00 = evaluateBezierPatch(u0, v0); vec3 p10 = evaluateBezierPatch(u1, v0);
            vec3 p01 = evaluateBezierPatch(u0, v1); vec3 p11 = evaluateBezierPatch(u1, v1);
            vec3 n1 = normalize(cross(p10 - p00, p01 - p00));
            vec3 n2 = normalize(cross(p01 - p11, p10 - p11));
            
            patchVertices.insert(patchVertices.end(), {p00.x,p00.y,p00.z, n1.x,n1.y,n1.z, u0, v0});
            patchVertices.insert(patchVertices.end(), {p10.x,p10.y,p10.z, n1.x,n1.y,n1.z, u1, v0});
            patchVertices.insert(patchVertices.end(), {p01.x,p01.y,p01.z, n1.x,n1.y,n1.z, u0, v1});
            
            patchVertices.insert(patchVertices.end(), {p10.x,p10.y,p10.z, n2.x,n2.y,n2.z, u1, v0});
            patchVertices.insert(patchVertices.end(), {p11.x,p11.y,p11.z, n2.x,n2.y,n2.z, u1, v1});
            patchVertices.insert(patchVertices.end(), {p01.x,p01.y,p01.z, n2.x,n2.y,n2.z, u0, v1});
        }
    }
    glBindVertexArray(patchVAO);
    glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
    glBufferData(GL_ARRAY_BUFFER, patchVertices.size() * sizeof(float), patchVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
}

// --- Full Helper Function Implementations ---
vec3 evaluateBezierCurve(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, float t) {
    float one_minus_t = 1.0f - t;
    return pow(one_minus_t, 3.0f) * p0 + 3.0f * pow(one_minus_t, 2.0f) * t * p1 + 3.0f * one_minus_t * pow(t, 2.0f) * p2 + pow(t, 3.0f) * p3;
}
vec3 evaluateBezierPatch(float u, float v) {
    vec3 v_curve_points[4];
    for (int i = 0; i < 4; ++i) {
        v_curve_points[i] = evaluateBezierCurve(controlPoints[i * 4 + 0], controlPoints[i * 4 + 1], controlPoints[i * 4 + 2], controlPoints[i * 4 + 3], u);
    }
    return evaluateBezierCurve(v_curve_points[0], v_curve_points[1], v_curve_points[2], v_curve_points[3], v);
}
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height); windowWidth = width; windowHeight = height;
}
string loadShaderFromFile(const string& filePath) {
    ifstream f(filePath);
    if (!f.is_open()) { cerr << "Failed to open shader file: " << filePath << endl; return ""; }
    stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type); glShaderSource(s, 1, &src, NULL); glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[1024]; glGetShaderInfoLog(s, 1024, NULL, log); cerr << "Shader Compile Error: " << log << endl; }
    return s;
}
GLuint makeProgram(const string& vPath, const string& fPath) {
    string vCode = loadShaderFromFile(vPath); string fCode = loadShaderFromFile(fPath);
    if(vCode.empty() || fCode.empty()){ cerr << "Shader file(s) not found or empty." << endl; return 0; }
    GLuint v = compileShader(GL_VERTEX_SHADER, vCode.c_str());
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fCode.c_str());
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v); glAttachShader(prog, f);
    glBindAttribLocation(prog, 0, "aPos");
    glBindAttribLocation(prog, 1, "aNormal");
    glBindAttribLocation(prog, 2, "aTexCoords");
    glLinkProgram(prog);
    GLint ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { char log[1024]; glGetProgramInfoLog(prog, 1024, NULL, log); cerr << "Link Error: " << log << endl; }
    glDeleteShader(v); glDeleteShader(f);
    return prog;
}
// --- END OF MODIFIED FILE ---
