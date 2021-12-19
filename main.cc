#include "glad.h" // must be before glfw.h
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <mikmod.h>
#include <vector>

constexpr int32_t SCREEN_WIDTH = 1600;
constexpr int32_t SCREEN_HEIGHT = 1100;
constexpr float fov = glm::radians(90.0f);
constexpr float LED_FLOOR = 60.0f;

constexpr auto vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
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

void camera(uint32_t shaderId) {
  glm::mat4 view = glm::mat4(1.0f);

  float zFar = (SCREEN_WIDTH / 2.0f) / tanf64(fov / 2.0f); // was 90.0f
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

std::vector<glm::vec3> generateLeds(uint32_t amount) {
  std::vector<glm::vec3> retVal;
  constexpr float newParameter = 16.0f;
  for (float y = 0; y < (float)amount; ++y) {
    for (float x = 0; x < (float)amount; ++x) {
      glm::vec3 vertex(SCREEN_WIDTH / 2.0 + (float)x * newParameter -
                           amount / 2.0f * newParameter,
                       SCREEN_HEIGHT / 2.0 + (float)y * newParameter -
                           amount / 2.0f * newParameter,
                       LED_FLOOR);
      std::cout << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;

      retVal.push_back(vertex);
    }
  }

  return retVal;
}

std::vector<float> generatedLedVertices(std::vector<float> &led,
                                        uint32_t amount) {
  std::vector<float> retVal;
  for (uint32_t i = 0; i < amount; i++) {
    for (auto coord : led) {
      retVal.push_back(coord);
    }
  }
  return retVal;
}

void dropLed(std::vector<glm::vec3> &leds, uint32_t amount, float zFar) {
  auto noLeds = leds.size();
  uint32_t iterations{0};
  uint32_t i{0};

  while (i < amount && iterations < noLeds) {
    iterations++;
    auto index = rand() % (noLeds - 1);
    if (leds[index].z < LED_FLOOR + 1.0f) {
      // std::cout << "dropping led at index " << index << std::endl;
      leds[index].z = zFar + rand() % (5 - 1);
      i++;
    }
  }
}

MODULE *initMikMod() {
  MODULE *module{nullptr};

  /* register all the drivers */
  MikMod_RegisterAllDrivers();

  /* register all the module loaders */
  MikMod_RegisterAllLoaders();

  /* initialize the library */
  md_mode |= DMODE_SOFT_MUSIC;
  if (MikMod_Init("")) {
    fprintf(stderr, "Could not initialize sound, reason: %s\n",
            MikMod_strerror(MikMod_errno));
    return nullptr;
  }
  module = Player_Load("resonance2.mod", 64, 0);
  return module;
}
int main() {
  std::vector<float> led = {
      0.5f,  0.5f, 0.0f, 0.5f,  -0.5f, 0.0f, -0.5f, 0.5f,  0.0f,

      -0.5f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f};

  auto leds = generateLeds(64);
  auto ledVertices = generatedLedVertices(led, 32);
  float deltaTime = 0.0f; // Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame

  srand(time(NULL));

  if (!glfwInit()) {
    // Initialization failed
    std::cerr << "Error could not init glfw!" << std::endl;
    exit(1);
  }

  glfwSetErrorCallback(error_callback);

  GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My Title",
                                        nullptr, nullptr);
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

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, ledVertices.size(), ledVertices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  auto vertexShader = loadShaders(vertexShaderSource, GL_VERTEX_SHADER);
  auto fragmentShader = loadShaders(fragmentShaderSource, GL_FRAGMENT_SHADER);
  auto shaderProgram = makeShaderProgram(vertexShader, fragmentShader);

  float zFar = (SCREEN_WIDTH / 2.0) / tanf64(fov / 2.0f) + 10.0f; // 100.0f
  glm::mat4 projection = glm::perspective(
      fov, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, zFar);

  glm::vec3 movement(0.0f, 0.0f, LED_FLOOR);
  glm::vec3 dz(0.0, 0.0, 1.0);
  /*
    auto module = initMikMod();
    if (module) {
       start module

    Player_Start(module);
    Player_SetVolume(32);
    }
*/
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);
    /*
        if (Player_Active()) {
          MikMod_Update();
        }
    */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

    // 2. use our shader program when we want to render an object
    glUseProgram(shaderProgram);

    int modelprj = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(modelprj, 1, GL_FALSE, glm::value_ptr(projection));

    camera(shaderProgram);

    glBindVertexArray(VAO);
    for (auto &poly : leds) {
      // std::cout<<poly.x<<" "<< poly.y<< " " <<poly.z<<std::endl;

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::rotate(model, glm::radians(-2.8f), glm::vec3(1.0, 0.0, 0.0));

      model = glm::translate(model, poly);
      model = glm::scale(model, glm::vec3(8.0, 8.0, 1.0));

      int modelLoc = glGetUniformLocation(shaderProgram, "model");
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

      dz.z = 64.0f * deltaTime / 1.0f;
      /*std::cout << "poly.z " << poly.z << " deltatime " << deltaTime
                << std::endl;
    */ // movement = movement + dz;
      if (poly.z > LED_FLOOR + 1.0f) {
        poly -= dz;
      }
      if (poly.z < LED_FLOOR) {
        poly.z = LED_FLOOR;
        std::cout << "resetting z axis" << std::endl;
      }

      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    dropLed(leds, 2, zFar);

    glfwSwapBuffers(window);
    // Keep running
    glfwPollEvents();
  }
  /*
    Player_Stop();
    Player_Free(module);
    MikMod_Exit();
  */
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
