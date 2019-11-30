
# CMake version
cmake_minimum_required(VERSION 3.10)

# Project
project(ImageInformationAnalyzer CXX)

# C++ Setting
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For OpenCV
find_package(OpenCV REQUIRED)

# For Eigen
find_package(Eigen3 REQUIRED)

# For glog
find_package(glog REQUIRED)

# For Ceres Solver
find_package(Ceres REQUIRED)

# AVX
option(USE_AVX "Use AVX" ON)
if(USE_AVX)
  if(MSVC)
    add_compile_options(/arch:AVX)
  else()
    add_compile_options(-mavx)
  endif() 
endif()

# OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Directories
add_subdirectory(src)
add_subdirectory(example)

