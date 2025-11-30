cmake_minimum_required(VERSION 3.10)
project(GazeAnaly LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(gaze_heatmap src/main.cpp)
target_link_libraries(gaze_heatmap PRIVATE ${OpenCV_LIBS})
