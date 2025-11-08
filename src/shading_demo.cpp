// --- START OF FINAL CORRECTED FILE my-project-master/src/shading_demo.cpp ---

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;
using namespace glm;

// --- Structs and Globals ---
struct Vertex { glm::vec3 pos; glm::vec3 normal; };
static std::vector<Vertex> g_vertices;
static std::vector<unsigned int> g_indices;
static GLuint g_VAO = 0, g_VBO = 0, g_EBO = 0;

static float camAngle = 0.0f, camRadius = 3.5f, camHeight = 0.0f;
static float lightAngle = 0.0f, lightRadius = 2.0f, lightHeight = 0.5f;
static bool g_perspective = true;
// --- MODIFIED: Fixed array declaration ---
static bool prevKeys[1024];

// --- Forward Declarations ---
static bool loadSMF(const std::string &filename, std::vector<glm::vec3> &positions, std::vector<glm::ivec3> &faces);
static GLuint makeProgram(const std::string& vPath, const std::string& fPath);
static bool buildMeshFromSMF(const std::string &path);
static void processContinuousInput(GLFWwindow* win);
std::string loadShaderFromFile(const std::string& filePath);
GLuint compileShader(GLenum type, const char* src);

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " models/your_model.smf\n"; return -1; }

    if (!glfwInit()) { std::cerr << "GLFW init fail\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(900, 700, "Part 3.2 - 3D Procedural Texture", NULL, NULL);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr << "GLAD init failed\n"; return -1; }

    if (!buildMeshFromSMF(argv[1])) { std::cerr << "Failed to build mesh\n"; return -1; }

    GLuint proceduralProgram = makeProgram("shaders/procedural_130.vert", "shaders/procedural_130.frag");

    glEnable(GL_DEPTH_TEST);
    for (int i=0; i<1024; ++i) prevKeys[i] = false;

    cout << "--- SMF Viewer with 3D Procedural Wood Texture ---\n"
         << "Controls from README work for Camera/Light/Projection.\n";

    while (!glfwWindowShouldClose(window)) {
        processContinuousInput(window);
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !prevKeys[GLFW_KEY_P]) {
            g_perspective = !g_perspective;
            std::cout << "Projection: " << (g_perspective ? "Perspective\n" : "Orthographic\n");
            prevKeys[GLFW_KEY_P] = true;
        } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
            prevKeys[GLFW_KEY_P] = false;
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, true); }

        glm::vec3 camPos(camRadius * cos(camAngle), camHeight, camRadius * sin(camAngle));
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        float aspect = (w > 0 && h > 0) ? (float)w / (float)h : 1.0f;
        glm::mat4 proj = g_perspective ? glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f)
                                       : glm::ortho(-camRadius * aspect, camRadius * aspect, -camRadius, camRadius, -100.0f, 100.0f);

        glm::vec3 worldLightPos(lightRadius * cos(lightAngle), lightHeight, lightRadius * sin(lightAngle));

        glClearColor(0.07f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(proceduralProgram);
        glUniformMatrix4fv(glGetUniformLocation(proceduralProgram, "model"), 1, GL_FALSE, glm::value_ptr(mat4(1.0f)));
        glUniformMatrix4fv(glGetUniformLocation(proceduralProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(proceduralProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(glGetUniformLocation(proceduralProgram, "viewPos"), 1, value_ptr(camPos));
        glUniform3fv(glGetUniformLocation(proceduralProgram, "lightPos"), 1, value_ptr(worldLightPos));

        glBindVertexArray(g_VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)g_indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// --- Helper function implementations ---
string loadShaderFromFile(const string& filePath) { ifstream f(filePath); if(!f.is_open()) return ""; stringstream ss; ss << f.rdbuf(); return ss.str(); }

// --- MODIFIED: Fixed char log to char log[1024] ---
GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type); glShaderSource(s, 1, &src, NULL); glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[1024]; glGetShaderInfoLog(s, 1024, NULL, log); cerr << "Shader Error: " << log << endl; }
    return s;
}

// --- MODIFIED: Fixed char log to char log[1024] ---
GLuint makeProgram(const string& vPath, const string& fPath) {
    string vCode = loadShaderFromFile(vPath); string fCode = loadShaderFromFile(fPath);
    if(vCode.empty() || fCode.empty()){ cerr << "Shader file(s) not found or empty." << endl; return 0; }
    GLuint v = compileShader(GL_VERTEX_SHADER, vCode.c_str());
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fCode.c_str());
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v); glAttachShader(prog, f);
    glBindAttribLocation(prog, 0, "aPos");
    glBindAttribLocation(prog, 1, "aNormal");
    glLinkProgram(prog);
    GLint ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) { char log[1024]; glGetProgramInfoLog(prog, 1024, NULL, log); cerr << "Link Error: " << log << endl; }
    glDeleteShader(v); glDeleteShader(f);
    return prog;
}

static bool loadSMF(const std::string &filename, std::vector<glm::vec3> &positions, std::vector<glm::ivec3> &faces) {
    ifstream in(filename);
    if (!in.is_open()) { cerr << "Cannot open SMF: " << filename << '\n'; return false; }
    string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        char type; ss >> type;
        if (type == 'v') { glm::vec3 p; ss >> p.x >> p.y >> p.z; positions.push_back(p); }
        else if (type == 'f') { glm::ivec3 f; ss >> f.x >> f.y >> f.z; faces.push_back(f - 1); }
    }
    return !positions.empty() && !faces.empty();
}

static bool buildMeshFromSMF(const std::string &path) {
    std::vector<glm::vec3> pos; std::vector<glm::ivec3> faces;
    if (!loadSMF(path, pos, faces)) return false;
    std::vector<glm::vec3> normals(pos.size(), glm::vec3(0.0f));
    for (auto &f : faces) {
        glm::vec3 v1 = pos[f.y] - pos[f.x], v2 = pos[f.z] - pos[f.x];
        glm::vec3 fn = glm::normalize(glm::cross(v1, v2));
        normals[f.x] += fn; normals[f.y] += fn; normals[f.z] += fn;
    }
    g_vertices.resize(pos.size());
    for (size_t i=0; i<pos.size(); ++i) { g_vertices[i] = {pos[i], glm::normalize(normals[i])}; }
    for (auto &f : faces) { g_indices.insert(g_indices.end(), { (unsigned int)f.x, (unsigned int)f.y, (unsigned int)f.z }); }
    glGenVertexArrays(1, &g_VAO); glGenBuffers(1, &g_VBO); glGenBuffers(1, &g_EBO);
    glBindVertexArray(g_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
    glBufferData(GL_ARRAY_BUFFER, g_vertices.size()*sizeof(Vertex), g_vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_indices.size()*sizeof(unsigned int), g_indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));
    glBindVertexArray(0);
    cout << "Loaded " << pos.size() << " vertices and " << faces.size() << " faces.\n";
    return true;
}

static void processContinuousInput(GLFWwindow* win) {
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) camAngle -= 0.02f;
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) camAngle += 0.02f;
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) camRadius = std::max(0.2f, camRadius - 0.04f);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) camRadius += 0.04f;
    if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS) camHeight += 0.04f;
    if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS) camHeight -= 0.04f;
    if (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS) lightAngle -= 0.02f;
    if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS) lightAngle += 0.02f;
    if (glfwGetKey(win, GLFW_KEY_I) == GLFW_PRESS) lightRadius -= 0.04f;
    if (glfwGetKey(win, GLFW_KEY_K) == GLFW_PRESS) lightRadius += 0.04f;
    if (glfwGetKey(win, GLFW_KEY_U) == GLFW_PRESS) lightHeight += 0.04f;
    if (glfwGetKey(win, GLFW_KEY_O) == GLFW_PRESS) lightHeight -= 0.04f;
}

// --- END OF MODIFIED FILE ---
