cmake_minimum_required(VERSION 3.10)
project(GameDevelopmentProject)

set(PROJECT_SOURCE_FILES
  source/main.cpp
)
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

include_directories(source)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

#Add GLFW Library
add_subdirectory(thirdparty/glfw-3.4
  "${CMAKE_CURRENT_BINARY_DIR}/glfw_build")
include_directories(thirdparty/glfw-3.4/inlcude)


#Add GLEW Library
set(BUILD_UTILS OFF CACHE BOOL "utilities" FORCE) #desabilitando GLEW utilities
add_subdirectory(thirdparty/glew/build/cmake
  "${CMAKE_CURRENT_BINARY_DIR}/glew_build")       #buildando glew via cmake scripts em glew_build/
include_directories(thirdparty/glew/include)      #adicionando .h do glew
set_property(TARGET glew_s PROPERTY FOLRER GLEW)
set_property(TARGET glew PROPERTY FOLDER GLEW)


#link todas a thirdparty bibliotecas
target_link_libraries(${PROJECT_NAME}
  glfw
  glew_s
  nvidia-ml
)
