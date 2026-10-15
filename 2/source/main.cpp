#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <unistd.h> 
#include <cmath>
#include  <map>
#include <random>
#include <chrono>
#include <vector>
#include <string>

struct Vec2
{
  float x = 0.0f;
  float y = 0.0;
};

Vec2 offset;

void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
  if (action == GLFW_PRESS)
  {
    switch(key)
    {
    case GLFW_KEY_UP:
        offset.y += 0.01f;
        break;
    case GLFW_KEY_DOWN:
        offset.y -= 0.01f;
        break;
    case GLFW_KEY_LEFT:
        offset.x += 0.01f;
        break;
    case GLFW_KEY_RIGHT:
        offset.x -= 0.01f;
        break;

    default:
        break;
    }
  }
}


int main() 
{
  
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); // Força o GLFW a usar o X11, para linux, caso o Wayland esteja ativo, para evitar problemas de compatibilidade com o OpenGL

  if (!glfwInit())
    return -1;

  //dizendo qual versão do OpenGL queremos usar, nesse caso a 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(720, 640, "GameDeveplomentProject", nullptr, nullptr);
  
  if (window == nullptr) 
  {
    std::cout << "Error creating window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwSetKeyCallback(window, keyCallback);

  //dizendo qual janela será associada ao contexto do OpenGL, nesse caso a janela criada pelo GLFW
  //glfwSetWindowPos(window, 2000, 150);
  glfwMakeContextCurrent(window);

  //Iniciando GLEW para usar as funções do OpenGL
  if(glewInit() != GLEW_OK)
  {
    std::cout << "Error initializing GLEW" << std::endl;
    glfwTerminate();
    return -1;
  }

  //definindo o vertex shader
 std::string vertexShaderSource = R"(
	#version 330 core
	layout(location = 0) in vec3 position;
  layout(location = 1) in vec3 color;

  uniform vec2 uOffset;

  out vec3 vColor;
	
  void main()
  {
    vColor = color;
		gl_Position = vec4(position.x + uOffset.x, position.y + uOffset.y, position.z, 1.0);
  }
  )";

//compilando shader
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // criando o vertex shader na GPU
  const char* vertexShaderCStr = vertexShaderSource.c_str(); //passando o nosso shader para o o vertex shader 
  glShaderSource(vertexShader, 1, &vertexShaderCStr, nullptr);
  glCompileShader(vertexShader);
  
  GLint success;

  //garantir que não ocorreu nennhum erro: 
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    std::cerr <<"ERROR: VERTEX_SHADER_COMPILATION_FAILED: " << infoLog << std::endl;
  }

  std::string fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 vColor;

    void main()
    {
      FragColor = vec4(vColor, 1.0);
    }
  )";

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
  glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr);
  glCompileShader(fragmentShader);
  
  //garantir que não ocorreu nennhum erro: 
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    std::cerr << "ERROR: FRAGMENT_SHADER_COMPILATION_FAILED: " << infoLog << std::endl;
  }
  //Criar o objeto do programa shader na GPU
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);    //anexando o vertex shader ao nosso shader program
  glAttachShader(shaderProgram, fragmentShader);  //anexando o fragment shader ao nosso shader program
  glLinkProgram(shaderProgram);

  //garantir que não ocorreu nennhum erro: 
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if(!success)
  {
    char infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
    std::cerr << "ERROR: SHADER_PROGRAM_LINKING_FAILED" << infoLog << std::endl;
  }

  // deletar os shader, pois o programa que linka os shader foi criado com sucesso
  // então não é mais necessário os shaders individuais
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glfwSetWindowPos(window, 1280, 540); // posição da janela, contabiliza todos os monitores

  std::vector<float> vertices = {
    0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom-left
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
     0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // Top-center
  };

  std::vector<float> quadrado =
  {
    0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom-left
    -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
    -0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // Top-center
    0.5f,  -0.5f, 0.0f, 1.0f, 1.0f, 0.0f  // Top-center

  };

  std::vector<unsigned int> indices =
  {
    0, 1, 2,
    0, 2, 3
  };

  // enviar os dados de vertices do nosso triangulo para a a memoria da GPU, usando buffers
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo); //indica que esse buffer vai armazenar um array de dados dos vertices
  glBufferData(
    GL_ARRAY_BUFFER, 
    vertices.size() * sizeof(float), 
    vertices.data(),
    GL_STATIC_DRAW //significa que envia uma vez e não o modifica, e vai ficar permanentemente na GPU
  ); //transferindo dados dos vertices da memoria do sistema para a memoria da GPU
  glBindBuffer(GL_ARRAY_BUFFER, 0); //Desfazando o bind do buffer, e os dados residirão apenas na memoria da GPU 
  //dizer ao shader como interpretar esse buffer, como deve ser lido pelo vertex shader
  //VAO armazena o layout dos atributos de vertices e como eles mapeiam para as entradas do shader

 
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  
  // Posição: 3 floats, começa no byte 0, a próxima posição está a 36 bytes
  // stride = 0 significa "empacotado" (o próximo elemento vem logo depois)
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0); 
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, false, 6*sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  GLint uColorLoc = glGetUniformLocation(shaderProgram, "uColor");
  GLint uOffsetLoc = glGetUniformLocation(shaderProgram, "uOffset");

  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.0f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);  
    /*
      A renderização é feita em um buffer, o backbuffer, e depois trocado com o frontbuffer, para evitar flickering.
      flickering é quando a tela pisca, isso acontece quando a renderização é feita diretamente no frontbuffer, que é o 
      que é mostrado na tela, e a renderização demora mais do que o tempo de atualização da tela, então a tela fica piscando, 
      mostrando frames incompletos
    */
    glUseProgram(shaderProgram);
    glUniform4f(uColorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
    glUniform2f(uOffsetLoc, offset.x, offset.y);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window); 

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}
