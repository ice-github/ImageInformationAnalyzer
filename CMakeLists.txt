
# CMake version
cmake_minimum_required(VERSION 2.8)

# Project
project(ImageInformationAnalyzer CXX)

# For OpenCV
find_package(OpenCV REQUIRED)

# Directories
add_subdirectory(src)
add_subdirectory(example)

