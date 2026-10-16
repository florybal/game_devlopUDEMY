cmake_minimum_required(VERSION 3.10)
project(GameDevelopmentProject CXX)

# ---- OpenGL preference (must be set BEFORE find_package) ----
set(OpenGL_GL_PREFERENCE GLVND)     # quotes not needed, but harmless

find_package(OpenGL REQUIRED)

# ---- Sources ----
set(PROJECT_SOURCE_FILES
  source/main.cpp
)

include_directories(source)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

# ---- GLFW ----
add_subdirectory(
  ../thirdparty/glfw-3.4
  "${CMAKE_CURRENT_BINARY_DIR}/glfw_build"
)
# fixed typo: "inlcude" -> "include"
include_directories(../thirdparty/glfw-3.4/include)

# ---- GLEW ----
set(BUILD_UTILS OFF CACHE BOOL "utilities" FORCE)
add_subdirectory(
  ../thirdparty/glew/build/cmake
  "${CMAKE_CURRENT_BINARY_DIR}/glew_build"
)
include_directories(../thirdparty/glew/include)

set_property(TARGET glew_s PROPERTY FOLDER GLEW)   # fixed typo
set_property(TARGET glew   PROPERTY FOLDER GLEW)

# ---- Link everything ----
target_link_libraries(${PROJECT_NAME}
  glfw
  glew_s
  OpenGL::GL          # ← this is the imported target from FindOpenGL
)
