cmake_minimum_required(VERSION 3.20)

project(FlappyBird)

set(LIB_HEAD externals/NeuralNetwork/Population/head externals/NeuralNetwork/Network/head externals/NeuralNetwork/Neuron/head externals/NeuralNetwork/Logging/head)

include_directories(head/bird head/game head/pipe externals/PixelGameEngine externals/NeuralNetwork/Population/head ${LIB_HEAD})

set(PIXELGAMEENGINE PixelGameEngine/)

set(GAME_SRC src/game/)
set(BIRD_SRC src/bird/)
set(PIPE_SRC src/pipe/)

add_subdirectory(${CMAKE_SOURCE_DIR}/externals/NeuralNetwork NeuralNetwork)
add_executable(${PROJECT_NAME} ${GAME_SRC}Main.cpp ${PIPE_SRC}Pipe.cpp ${BIRD_SRC}Bird.cpp)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC NeuralNetwork ${OPENGL_LIBRARIES} ${X11_LIBRARIES} ${PNG_LIBRARIES} Threads::Threads )

#target_link_libraries(${PROJECT_NAME} PUBLIC NeuralNetwork)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)