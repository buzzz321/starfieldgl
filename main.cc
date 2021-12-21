// clang-format off
#include "glad.h" // must be before glfw.h
#include <GLFW/glfw3.h>
// clang-format on
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>
#include <vector>

constexpr int32_t SCREEN_WIDTH = 1600;
constexpr int32_t SCREEN_HEIGHT = 1100;
constexpr float fov = glm::radians(90.0f);
constexpr float LED_FLOOR = 60.0f;

constexpr auto vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos+aOffset, 1.0);
}
)";

constexpr auto fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} )";

unsigned int loadShaders(const char *shaderSource, GLenum shaderType) {
  unsigned int shader{0};
  int success{0};
  char infoLog[1024];

  shader = glCreateShader(shaderType); // GL_VERTEX_SHADER

  glShaderSource(shader, 1, &shaderSource, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  return shader;
}

unsigned int makeShaderProgram(uint32_t vertexShader, uint32_t fragmentShader) {
  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

void error_callback(int error, const char *description) {
  std::cerr << "Error: " << description << " error number " << error
            << std::endl;
}

void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action,
                  int /*mods*/) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void camera(uint32_t shaderId, float dist) {
  glm::mat4 view = glm::mat4(1.0f);

  float zFar =
    2 + dist; //(SCREEN_WIDTH / 2.0f) / tanf64(fov / 2.0f); // was 90.0f
  glm::vec3 cameraPos =
    glm::vec3(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, zFar);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  //  std::cout << " x= " << cameraPos.x << " y = " << cameraPos.y
  //            << " z = " << cameraPos.z << " zFar = " << zFar << " tan µ "
  //            << tanf64(fov / 2.0f) << " fov " << fov << std::endl;
  // glm::vec3 cameraFront = glm::vec3(32.0f, 32.0f, -1.0f);

  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

  int modelView = glGetUniformLocation(shaderId, "view");
  glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));
}

std::vector<glm::vec3> generateStarOffsets(uint32_t amount) {
  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_int_distribution<int> xrand(0, (float)SCREEN_WIDTH);
  std::uniform_int_distribution<int> yrand(0, (float)SCREEN_HEIGHT);
  std::uniform_int_distribution<int> zrand(LED_FLOOR, 100.0f);
  std::vector<glm::vec3> retVal;

  for (float index = 0; index < (float)amount; ++index) {
    glm::vec3 vertex((float)xrand(e1), (float)yrand(e1), (float)zrand(e1));
    std::cout << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;

    retVal.push_back(vertex);
  }

  return retVal;
}

int main() {
  std::srand(
    std::time(nullptr)); // use current time as seed for random generator
  int random_variable = std::rand();
  // clang-format off
  std::vector<float> star = {
      -0.5f, -0.5f, 0.0f,
       0.5f, -0.5f, 0.0f,
       0.5f,  0.5f, 0.0f,
      -0.5f, -0.5f, 0.0f,
       0.5f,  0.5f, 0.0f,
      -0.5f,  0.5f, 0.0f
  };
  // clang-format on

  auto starOffsets = generateStarOffsets(1000);
  float deltaTime = 0.0f; // Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame

  srand(time(NULL));

  if (!glfwInit()) {
    // Initialization failed
    std::cerr << "Error could not init glfw!" << std::endl;
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  glfwSetErrorCallback(error_callback);

  GLFWwindow *window =
    glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My Title", nullptr, nullptr);
  if (!window) {
    std::cerr << "Error could not create window" << std::endl;
    exit(1);
    // Window or OpenGL context creation failed
  }

  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << " Error could not load glad " << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(1);
  }
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);

  // verticies
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, star.size() * sizeof(float), &star[0],
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // end verticies

  // instance data
  unsigned int instanceVBO;
  glGenBuffers(1, &instanceVBO);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, starOffsets.size() * sizeof(glm::vec3),
               &starOffsets[0], GL_STATIC_DRAW);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // end instance data

  auto vertexShader = loadShaders(vertexShaderSource, GL_VERTEX_SHADER);
  auto fragmentShader = loadShaders(fragmentShaderSource, GL_FRAGMENT_SHADER);
  auto shaderProgram = makeShaderProgram(vertexShader, fragmentShader);

  float zFar = (SCREEN_WIDTH / 2.0) / tanf64(fov / 2.0f) + 10.0f; // 100.0f
  glm::mat4 projection = glm::perspective(
    fov, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, zFar);

  glm::vec3 dz(0.0, 0.0, 1.0);
  float dist = 0.0f;
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

    // 2. use our shader program when we want to render an object
    glUseProgram(shaderProgram);

    int modelprj = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(modelprj, 1, GL_FALSE, glm::value_ptr(projection));

    camera(shaderProgram, dist);
    dist += 1.0f;

    glBindVertexArray(VAO);
    // for (auto &poly : starOffsets) {
    {
      auto poly = starOffsets[0];
      // std::cout<<poly.x<<" "<< poly.y<< " " <<poly.z<<std::endl;

      glm::mat4 model = glm::mat4(1.0f);
      // model = glm::rotate(model, glm::radians(-2.8f), glm::vec3(1.0, 0.0,
      // 0.0));

      // model = glm::translate(model, glm::vec3(1.0, 1.0, 60.0));
      // model = glm::scale(model, glm::vec3(8.0, 8.0, 1.0));

      int modelLoc = glGetUniformLocation(shaderProgram, "model");
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

      dz.z = 64.0f * deltaTime / 1.0f;
      /*std::cout << "poly.z " << poly.z << " deltatime " << deltaTime
                << std::endl;
    */
      glDrawArraysInstanced(GL_TRIANGLES, 0, 6, starOffsets.size());
      // glBindVertexArray(0);
    }

    glfwSwapBuffers(window);
    // Keep running
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
