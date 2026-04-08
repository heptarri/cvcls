cmake_minimum_required(VERSION 3.10)
project(CVCLS VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find Eigen3 (required for double_measure)
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Common library
add_library(common STATIC
    src/common/common.cpp
    src/common/common.h
)
target_include_directories(common PUBLIC src/common)
target_link_libraries(common ${OpenCV_LIBS})

# Main executable (all chapters integrated)
add_executable(cvcls
    src/main.cpp
    src/chapts/chapt2/chapt2.cpp
    src/chapts/chapt2/chapt2.h
    src/chapts/chapt3/chapt3.cpp
    src/chapts/chapt3/chapt3.h
    src/chapts/chapt4/chapt4.cpp
    src/chapts/chapt4/chapt4.h
    src/chapts/chapt5/chapt5.cpp
    src/chapts/chapt5/chapt5.h
    src/chapts/others/single_measure.cpp
    src/chapts/others/single_measure.h
    src/chapts/others/stereo_measure.cpp
    src/chapts/others/stereo_measure.h
)
target_include_directories(cvcls PRIVATE 
    src
    src/chapts/chapt2
    src/chapts/chapt3
    src/chapts/chapt4
    src/chapts/chapt5
    src/chapts/others
)
target_link_libraries(cvcls common ${OpenCV_LIBS})

# Print build information
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
