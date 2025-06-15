#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <cmath>
#include "tinyobj/tiny_obj_loader.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

float roll_angle = 0.0f;
float pitch_angle = 0.0f;
float yaw_angle = 0.0f;

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

struct Object {
    GLuint program;
    GLuint vao;
    GLsizei count;
};

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("failed to open file!");
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

Object initObj() {
    Object obj{};

    const std::string vertexString = readFile("shader/shader.vert");
    const std::string fragmentString = readFile("shader/shader.frag");
    //shaderlar derlenir
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertexText = vertexString.c_str();
    glShaderSource(vertexShader, 1, &vertexText, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragmentText = fragmentString.c_str();
    glShaderSource(fragmentShader, 1, &fragmentText, nullptr);
    glCompileShader(fragmentShader);

    obj.program = glCreateProgram();
    glAttachShader(obj.program, vertexShader);
    glAttachShader(obj.program, fragmentShader);
    glLinkProgram(obj.program);
    glValidateProgram(obj.program);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models/11803_Airplane_v1_l1.obj");

    if (!ret) {
        std::cerr << "TinyObjLoader error: " << err << std::endl;
        throw std::runtime_error("Failed to load airplane model");
    }

    std::vector<Vertex> vertices;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex v{};
            v.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            v.color = glm::vec3(0.0f, 0.0f, 0.0f); // siyah renk
            vertices.push_back(v);
        }
    }
    obj.count = vertices.size();

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &obj.vao);
    glBindVertexArray(obj.vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return obj;
}
// x ve y eksenindeki cizgiler
Object initAxes(GLuint sharedProgram) {
    Object obj{};

    std::vector<Vertex> vertices = {
        // X ekseni - kırmızı
        {{0.0f, 0.0f, 0.0f}, {1, 0, 0}},
        {{2.0f, 0.0f, 0.0f}, {1, 0, 0}},

        // Y ekseni - yeşil
        {{0.0f, 0.0f, 0.0f}, {0, 1, 0}},
        {{0.0f, 2.0f, 0.0f}, {0, 1, 0}},

        // Z ekseni - mavi
        {{0.0f, 0.0f, 0.0f}, {0, 0, 1}},
        {{0.0f, 0.0f, 2.0f}, {0, 0, 1}},
    };

    obj.count = vertices.size();

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &obj.vao);
    glBindVertexArray(obj.vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    obj.program = sharedProgram;
    return obj;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) roll_angle += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) roll_angle -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pitch_angle += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pitch_angle -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) yaw_angle += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) yaw_angle -= 0.5f;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    roll_angle = 0.0f;
    pitch_angle = 0.0f;
    yaw_angle = 0.0f;
    }

    roll_angle = fmod(roll_angle, 360.0f);
    pitch_angle = fmod(pitch_angle, 360.0f);
    yaw_angle = fmod(yaw_angle, 360.0f);
}

void draw(Object obj, Object axes) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(obj.program);

    GLint modelLoc = glGetUniformLocation(obj.program, "uModel");
    GLint viewLoc = glGetUniformLocation(obj.program, "uView");
    GLint projLoc = glGetUniformLocation(obj.program, "uProj");

    glm::mat4 view = glm::lookAt(glm::vec3(0, 2, -6), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);

    glm::vec3 pivot(0.35f, 0.0f, 0.0f); // !!!!!! UCAGIN HAREKETLERININ MERKEZINI DEGISTIRMEK ISTIYORSAN BU POVOT KONUMUNU DEGISTIRMELISIN !!!!!

    glm::mat4 model = glm::mat4(1.0f);

    // Pivot merkezli dönüş
    model = glm::translate(model, -pivot);
    model = glm::rotate(model, glm::radians(roll_angle), glm::vec3(0, 0, 1));
    model = glm::rotate(model, glm::radians(pitch_angle), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(yaw_angle), glm::vec3(0, 1, 0));
    model = glm::translate(model, pivot);

    // Modeli dikleştir ve ölçekle
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.0005f)); // MODELI BUYUTMEK VEYA KUCULTMEK ICIN BU DEGERI DEGISTIRMELISIN


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(obj.vao);
    glDrawArrays(GL_TRIANGLES, 0, obj.count);

    glm::mat4 axisModel = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &axisModel[0][0]);
    glBindVertexArray(axes.vao);
    glDrawArrays(GL_LINES, 0, axes.count);
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Roll Pitch Yaw Airplane", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwMakeContextCurrent(window);
    glewInit();

    Object obj = initObj();
    Object axes = initAxes(obj.program);

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);// ARKAPLAN RENGINI DEGISTIRMEN ICIN BU DEGERLERLE OYNAMAN GEREKLI
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        draw(obj, axes);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}