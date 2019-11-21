
# CMake version
cmake_minimum_required(VERSION 2.8)

# Project
project(ImageInformationAnalyzer CXX)

# For OpenCV
find_package(OpenCV REQUIRED)

# For Eigen
find_package(Eigen3 REQUIRED)

# For glog
find_package(glog REQUIRED)

# For Ceres Solver
find_package(Ceres REQUIRED)



# Directories
add_subdirectory(src)
add_subdirectory(example)

